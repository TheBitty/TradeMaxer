#include "trading_engine.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>

namespace TradingSystem {

TradingEngine::TradingEngine(TradingMode mode, double initial_balance)
    : mode(mode), initial_balance(initial_balance), peak_balance(initial_balance),
      max_position_size(5000.0), max_drawdown(0.20), 
      stop_loss_percentage(0.02), take_profit_percentage(0.05),
      order_counter(0) {
    
    portfolio.cash_balance = initial_balance;
    portfolio.total_value = initial_balance;
}

TradingEngine::~TradingEngine() {
}

bool TradingEngine::initialize(std::shared_ptr<DatabaseManager> db_manager) {
    this->db_manager = db_manager;
    
    // Load existing positions from database
    auto positions = db_manager->getOpenPositions();
    for (const auto& pos : positions) {
        portfolio.positions[pos.symbol] = pos;
    }
    
    return true;
}

std::string TradingEngine::generateOrderId() {
    int order_num = ++order_counter;
    return "ORD_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(order_num);
}

std::string TradingEngine::placeOrder(const std::string& symbol, 
                                     const std::string& side,
                                     double quantity,
                                     const std::string& order_type,
                                     double price) {
    
    // Validate order
    if (quantity <= 0) {
        std::cerr << "Invalid order quantity: " << quantity << std::endl;
        return "";
    }
    
    // Check risk limits
    if (!checkRiskLimits(symbol, quantity, price)) {
        std::cerr << "Order rejected due to risk limits" << std::endl;
        return "";
    }
    
    // Create order
    Order order;
    order.order_id = generateOrderId();
    order.symbol = symbol;
    order.side = side;
    order.quantity = quantity;
    order.price = price;
    order.order_type = order_type;
    order.status = "PENDING";
    order.timestamp = std::chrono::system_clock::now();
    
    // Store order
    {
        std::lock_guard<std::mutex> lock(order_mutex);
        pending_orders[order.order_id] = order;
    }
    
    // Save to database
    if (db_manager) {
        db_manager->insertOrder(order);
    }
    
    // In paper trading mode, simulate immediate fill for market orders
    if (mode == TradingMode::PAPER && order_type == "MARKET") {
        simulateFill(order, price > 0 ? price : 100000.0); // Use provided price or default
    }
    
    return order.order_id;
}

bool TradingEngine::checkRiskLimits(const std::string& symbol, double quantity, double price) {
    // Check position size limit
    double position_value = quantity * price;
    if (position_value > max_position_size) {
        std::cerr << "Position size " << position_value << " exceeds limit " << max_position_size << std::endl;
        return false;
    }
    
    // Check if we have enough cash
    if (position_value > portfolio.cash_balance) {
        std::cerr << "Insufficient funds. Required: " << position_value << ", Available: " << portfolio.cash_balance << std::endl;
        return false;
    }
    
    // Check drawdown limit
    double current_drawdown = (peak_balance - portfolio.getEquity()) / peak_balance;
    if (current_drawdown > max_drawdown) {
        std::cerr << "Maximum drawdown exceeded: " << current_drawdown << std::endl;
        return false;
    }
    
    return true;
}

void TradingEngine::simulateFill(Order& order, double market_price) {
    PaperTradingSimulator simulator;
    
    // Apply slippage and spread
    double fill_price = simulator.simulateSlippage(market_price, order.quantity, order.side);
    fill_price = simulator.simulateSpread(fill_price);
    
    // Execute order
    if (order.order_type == "MARKET") {
        executeMarketOrder(order, fill_price);
    } else if (order.order_type == "LIMIT") {
        if (simulator.simulateOrderFill(order, market_price)) {
            executeLimitOrder(order, order.price);
        }
    }
}

bool TradingEngine::executeMarketOrder(Order& order, double market_price) {
    order.price = market_price;
    order.status = "FILLED";
    
    // Update portfolio
    updatePortfolio(order);
    
    // Move from pending to filled
    {
        std::lock_guard<std::mutex> lock(order_mutex);
        pending_orders.erase(order.order_id);
        filled_orders[order.order_id] = order;
    }
    
    // Update database
    if (db_manager) {
        db_manager->updateOrderStatus(order.order_id, "FILLED");
    }
    
    std::cout << "Order " << order.order_id << " filled at " << market_price << std::endl;
    return true;
}

void TradingEngine::updatePortfolio(const Order& order) {
    if (order.side == "BUY") {
        // Deduct cash
        portfolio.cash_balance -= order.quantity * order.price;
        
        // Add or update position
        if (portfolio.positions.find(order.symbol) != portfolio.positions.end()) {
            Position& pos = portfolio.positions[order.symbol];
            double total_cost = (pos.quantity * pos.entry_price) + (order.quantity * order.price);
            pos.quantity += order.quantity;
            pos.entry_price = total_cost / pos.quantity; // Average price
            pos.current_price = order.price;
        } else {
            Position pos;
            pos.symbol = order.symbol;
            pos.quantity = order.quantity;
            pos.entry_price = order.price;
            pos.current_price = order.price;
            pos.entry_time = std::chrono::system_clock::now();
            pos.unrealized_pnl = 0.0;
            portfolio.positions[order.symbol] = pos;
            
            // Save to database
            if (db_manager) {
                db_manager->insertPosition(pos);
            }
        }
    } else if (order.side == "SELL") {
        if (portfolio.positions.find(order.symbol) != portfolio.positions.end()) {
            Position& pos = portfolio.positions[order.symbol];
            
            // Calculate realized P&L
            double realized_pnl = order.quantity * (order.price - pos.entry_price);
            portfolio.cash_balance += order.quantity * order.price;
            
            // Update position
            pos.quantity -= order.quantity;
            if (pos.quantity <= 0) {
                portfolio.positions.erase(order.symbol);
            }
            
            // Track peak balance for drawdown calculation
            double current_equity = portfolio.getEquity();
            if (current_equity > peak_balance) {
                peak_balance = current_equity;
            }
        }
    }
}

void TradingEngine::processTradingSignal(const TradingSignal& signal) {
    std::cout << "Processing signal for " << signal.symbol 
              << ": " << signal.action 
              << " (confidence: " << signal.confidence << ")" << std::endl;
    
    // Calculate position size based on confidence and risk parameters
    double position_size = calculatePositionSize(signal);
    
    if (position_size <= 0) {
        std::cout << "Position size too small, skipping signal" << std::endl;
        return;
    }
    
    // Get current position if any
    auto pos_it = portfolio.positions.find(signal.symbol);
    bool has_position = (pos_it != portfolio.positions.end() && pos_it->second.quantity > 0);
    
    // Execute based on signal
    if (signal.action == "BUY" && !has_position) {
        // Calculate quantity based on position size and assumed price
        double quantity = position_size / 100000.0; // Assuming BTC price around 100k
        placeOrder(signal.symbol, "BUY", quantity, "MARKET");
        
    } else if (signal.action == "SELL" && has_position) {
        // Sell existing position
        placeOrder(signal.symbol, "SELL", pos_it->second.quantity, "MARKET");
        
    } else if (signal.action == "HOLD") {
        std::cout << "Holding position for " << signal.symbol << std::endl;
    }
}

double TradingEngine::calculatePositionSize(const TradingSignal& signal) {
    // Base position size on confidence and available capital
    double available_capital = portfolio.cash_balance;
    double base_size = signal.suggested_position_size;
    
    // Adjust for confidence
    double adjusted_size = base_size * signal.confidence;
    
    // Ensure we don't exceed available capital
    adjusted_size = std::min(adjusted_size, available_capital * 0.95); // Keep 5% reserve
    
    // Ensure we don't exceed max position size
    adjusted_size = std::min(adjusted_size, max_position_size);
    
    return adjusted_size;
}

void TradingEngine::updatePositionPrices(const std::map<std::string, double>& current_prices) {
    for (auto& [symbol, position] : portfolio.positions) {
        auto price_it = current_prices.find(symbol);
        if (price_it != current_prices.end()) {
            position.current_price = price_it->second;
            position.unrealized_pnl = position.quantity * (position.current_price - position.entry_price);
            
            // Check stop loss and take profit
            applyStopLoss(position, position.current_price);
            applyTakeProfit(position, position.current_price);
            
            // Update in database
            if (db_manager) {
                db_manager->updatePosition(position);
            }
        }
    }
    
    // Update portfolio total value
    portfolio.total_value = portfolio.getEquity();
}

void TradingEngine::applyStopLoss(Position& position, double current_price) {
    double loss_percentage = (position.entry_price - current_price) / position.entry_price;
    
    if (loss_percentage >= stop_loss_percentage) {
        std::cout << "Stop loss triggered for " << position.symbol 
                  << " at " << current_price << std::endl;
        placeOrder(position.symbol, "SELL", position.quantity, "MARKET", current_price);
    }
}

void TradingEngine::applyTakeProfit(Position& position, double current_price) {
    double profit_percentage = (current_price - position.entry_price) / position.entry_price;
    
    if (profit_percentage >= take_profit_percentage) {
        std::cout << "Take profit triggered for " << position.symbol 
                  << " at " << current_price << std::endl;
        placeOrder(position.symbol, "SELL", position.quantity, "MARKET", current_price);
    }
}

Portfolio TradingEngine::getPortfolio() const {
    return portfolio;
}

Position TradingEngine::getPosition(const std::string& symbol) {
    auto it = portfolio.positions.find(symbol);
    if (it != portfolio.positions.end()) {
        return it->second;
    }
    return Position();
}

std::vector<Position> TradingEngine::getAllPositions() {
    std::vector<Position> positions;
    for (const auto& [symbol, pos] : portfolio.positions) {
        positions.push_back(pos);
    }
    return positions;
}

double TradingEngine::getTotalPnL() const {
    double total_pnl = portfolio.getEquity() - initial_balance;
    return total_pnl;
}

double TradingEngine::getWinRate() const {
    if (db_manager) {
        int wins = db_manager->getWinningTrades();
        int losses = db_manager->getLosingTrades();
        int total = wins + losses;
        
        if (total > 0) {
            return static_cast<double>(wins) / total;
        }
    }
    return 0.0;
}

// PaperTradingSimulator implementation
PaperTradingSimulator::PaperTradingSimulator() 
    : slippage_rate(0.001), spread_rate(0.0005), fill_probability(0.95) {
}

double PaperTradingSimulator::simulateSlippage(double price, double quantity, const std::string& side) {
    // Simulate slippage based on order size
    double slippage = price * slippage_rate * (1.0 + quantity / 100.0);
    
    if (side == "BUY") {
        return price + slippage; // Pay more when buying
    } else {
        return price - slippage; // Receive less when selling
    }
}

double PaperTradingSimulator::simulateSpread(double price) {
    // Add bid-ask spread
    return price * (1.0 + spread_rate);
}

bool PaperTradingSimulator::simulateOrderFill(const Order& order, double market_price) {
    // For limit orders, check if price would be hit
    if (order.order_type == "LIMIT") {
        if (order.side == "BUY" && order.price >= market_price) {
            // Buy limit order fills if market price drops to or below limit
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            return dis(gen) < fill_probability;
        } else if (order.side == "SELL" && order.price <= market_price) {
            // Sell limit order fills if market price rises to or above limit
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            return dis(gen) < fill_probability;
        }
    }
    
    return order.order_type == "MARKET"; // Market orders always fill
}

} // namespace TradingSystem