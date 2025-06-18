#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <atomic>
#include <memory>
#include <vector>
#include <sstream>

#include "api.h"
#include "config/config_manager.h"
#include "logging/logger.h"
#include "database/database_manager.h"
#include "ipc/ipc_manager.h"
#include "trading/trading_engine.h"
#include "common/data_types.h"

using namespace TradingSystem;

std::atomic<bool> running(true);

void signalHandler(int signum) {
    LOG_INFO("Received signal " + std::to_string(signum) + ", shutting down...");
    running = false;
}

class TradingSystemApp {
private:
    std::unique_ptr<StockApi> api;
    std::shared_ptr<DatabaseManager> db_manager;
    std::unique_ptr<IPCManager> ipc_manager;
    std::unique_ptr<PythonProcessManager> python_manager;
    std::unique_ptr<TradingEngine> trading_engine;
    TradingConfig config;
    
public:
    bool initialize() {
        // Set up signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        // Load configuration
        LOG_INFO("Loading configuration...");
        ConfigManager::getInstance().loadConfig("config.ini");
        config = TradingConfig::loadFromConfig();
        
        // Initialize logger
        Logger::getInstance().initialize(config.log_file, LogLevel::INFO);
        Logger::getInstance().setLogLevel(config.log_level);
        LOG_INFO("Trading System starting...");
        
        // Initialize database
        LOG_INFO("Initializing database...");
        db_manager = std::make_shared<DatabaseManager>(config.db_path);
        if (!db_manager->initialize()) {
            LOG_ERROR("Failed to initialize database");
            return false;
        }
        
        // Initialize API
        LOG_INFO("Initializing API connection...");
        api = std::make_unique<StockApi>();
        if (!api->isInitialized()) {
            LOG_ERROR("Failed to initialize API - check API_KEY in .env file");
            return false;
        }
        
        // Initialize IPC
        LOG_INFO("Setting up IPC communication...");
        ipc_manager = std::make_unique<IPCManager>(config.ipc_pipe_name);
        if (!ipc_manager->initialize()) {
            LOG_ERROR("Failed to initialize IPC");
            return false;
        }
        
        // Set up message callback
        ipc_manager->setMessageCallback([this](const std::string& message) {
            handlePythonMessage(message);
        });
        
        // Start IPC
        ipc_manager->start();
        
        // Initialize Python process
        LOG_INFO("Starting Python analyzer process...");
        python_manager = std::make_unique<PythonProcessManager>("market_data_analyzer.py");
        python_manager->attachIPC(ipc_manager.get());
        
        if (!python_manager->start()) {
            LOG_ERROR("Failed to start Python process");
            return false;
        }
        
        // Wait for Python to be ready
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Initialize trading engine
        LOG_INFO("Initializing trading engine...");
        TradingMode mode = (config.trading_mode == "live") ? 
                          TradingMode::LIVE : TradingMode::PAPER;
        
        trading_engine = std::make_unique<TradingEngine>(mode, config.initial_balance);
        trading_engine->initialize(db_manager);
        trading_engine->setMaxPositionSize(config.max_position_size);
        trading_engine->setMaxDrawdown(config.max_drawdown);
        
        LOG_INFO("System initialization complete");
        return true;
    }
    
    void run() {
        LOG_INFO("Trading system running in " + config.trading_mode + " mode");
        LOG_INFO("Initial balance: $" + std::to_string(config.initial_balance));
        LOG_INFO("Tracking symbols: " + vectorToString(config.symbols));
        
        auto last_fetch = std::chrono::steady_clock::now();
        auto last_analysis = std::chrono::steady_clock::now();
        
        while (running) {
            auto now = std::chrono::steady_clock::now();
            
            // Fetch market data at intervals
            if (std::chrono::duration_cast<std::chrono::seconds>
                (now - last_fetch).count() >= config.data_fetch_interval_seconds) {
                
                fetchMarketData();
                last_fetch = now;
            }
            
            // Run analysis at intervals
            if (std::chrono::duration_cast<std::chrono::seconds>
                (now - last_analysis).count() >= config.analysis_interval_seconds) {
                
                runAnalysis();
                last_analysis = now;
            }
            
            // Update portfolio positions with current prices
            updatePortfolio();
            
            // Display status
            displayStatus();
            
            // Sleep for a bit
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    void shutdown() {
        LOG_INFO("Shutting down trading system...");
        
        // Stop Python process
        if (python_manager) {
            python_manager->stop();
        }
        
        // Stop IPC
        if (ipc_manager) {
            ipc_manager->stop();
        }
        
        // Final portfolio status
        if (trading_engine) {
            Portfolio portfolio = trading_engine->getPortfolio();
            LOG_INFO("Final portfolio value: $" + 
                    std::to_string(portfolio.getEquity()));
            LOG_INFO("Total P&L: $" + 
                    std::to_string(trading_engine->getTotalPnL()));
        }
        
        LOG_INFO("Shutdown complete");
    }
    
private:
    void fetchMarketData() {
        PerformanceTimer timer("FetchMarketData");
        
        for (const auto& symbol : config.symbols) {
            try {
                LOG_DEBUG("Fetching data for " + symbol);
                
                // For now, we're using BTC price for all symbols
                // In production, you'd fetch each symbol's data
                CryptoPrice price = api->getBTCPrice();
                
                // Convert to MarketData
                MarketData data;
                data.symbol = symbol;
                data.open = price.open;
                data.high = price.high;
                data.low = price.low;
                data.close = price.price;
                data.volume = price.volume;
                data.timestamp = std::chrono::system_clock::now();
                
                // Save to database
                db_manager->insertMarketData(data);
                
                LOG_INFO("Fetched " + symbol + " price: $" + 
                        std::to_string(price.price));
                
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to fetch data for " + symbol + ": " + e.what());
            }
        }
    }
    
    void runAnalysis() {
        PerformanceTimer timer("RunAnalysis");
        LOG_INFO("Running market analysis...");
        
        // Create analysis request
        std::stringstream ss;
        ss << "{\"command\":\"batch_analyze\",\"symbols\":[";
        for (size_t i = 0; i < config.symbols.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "\"" << config.symbols[i] << "\"";
        }
        ss << "]}";
        
        // Send to Python analyzer
        if (!ipc_manager->sendMessage(ss.str())) {
            LOG_ERROR("Failed to send analysis request");
        }
    }
    
    void handlePythonMessage(const std::string& message) {
        LOG_DEBUG("Received from Python: " + message);
        
        try {
            // Parse JSON response (simplified parsing)
            if (message.find("\"error\"") != std::string::npos) {
                LOG_ERROR("Python error: " + message);
                return;
            }
            
            // Look for trading signals
            if (message.find("\"action\"") != std::string::npos) {
                TradingSignal signal = TradingSignal::fromJson(message);
                
                LOG_INFO("Received signal: " + signal.symbol + " " + 
                        signal.action + " (confidence: " + 
                        std::to_string(signal.confidence) + ")");
                
                // Process signal through trading engine
                trading_engine->processTradingSignal(signal);
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error processing Python message: " + std::string(e.what()));
        }
    }
    
    void updatePortfolio() {
        // Get current prices for all positions
        std::map<std::string, double> current_prices;
        
        for (const auto& symbol : config.symbols) {
            auto market_data = db_manager->getMarketData(symbol, 1);
            if (!market_data.empty()) {
                current_prices[symbol] = market_data[0].close;
            }
        }
        
        // Update trading engine with current prices
        trading_engine->updatePositionPrices(current_prices);
    }
    
    void displayStatus() {
        static int counter = 0;
        if (++counter % 30 == 0) { // Display every 30 seconds
            Portfolio portfolio = trading_engine->getPortfolio();
            
            LOG_INFO("=== Portfolio Status ===");
            LOG_INFO("Cash: $" + std::to_string(portfolio.cash_balance));
            LOG_INFO("Total Equity: $" + std::to_string(portfolio.getEquity()));
            LOG_INFO("P&L: $" + std::to_string(trading_engine->getTotalPnL()));
            
            auto positions = trading_engine->getAllPositions();
            if (!positions.empty()) {
                LOG_INFO("Open Positions:");
                for (const auto& pos : positions) {
                    LOG_INFO("  " + pos.symbol + ": " + 
                            std::to_string(pos.quantity) + " @ $" + 
                            std::to_string(pos.entry_price) + 
                            " (P&L: " + std::to_string(pos.getPnlPercentage()) + "%)");
                }
            }
        }
    }
    
    std::string vectorToString(const std::vector<std::string>& vec) {
        std::stringstream ss;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << vec[i];
        }
        return ss.str();
    }
};

int main() {
    TradingSystemApp app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize trading system" << std::endl;
        return 1;
    }
    
    app.run();
    app.shutdown();
    
    return 0;
}