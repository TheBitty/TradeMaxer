#ifndef TRADING_ENGINE_H
#define TRADING_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include "../common/data_types.h"
#include "../database/database_manager.h"

namespace TradingSystem {

enum class TradingMode {
    PAPER,      // Paper trading (simulated)
    LIVE        // Live trading (real money)
};

struct Portfolio {
    double cash_balance;
    double total_value;
    std::map<std::string, Position> positions;
    
    double getEquity() const {
        double equity = cash_balance;
        for (const auto& [symbol, position] : positions) {
            equity += position.quantity * position.current_price;
        }
        return equity;
    }
};

class TradingEngine {
public:
    TradingEngine(TradingMode mode = TradingMode::PAPER, 
                  double initial_balance = 10000.0);
    ~TradingEngine();
    
    // Initialize engine
    bool initialize(std::shared_ptr<DatabaseManager> db_manager);
    
    // Order management
    std::string placeOrder(const std::string& symbol, 
                          const std::string& side,
                          double quantity,
                          const std::string& order_type = "MARKET",
                          double price = 0.0);
    
    bool cancelOrder(const std::string& order_id);
    Order getOrderStatus(const std::string& order_id);
    
    // Position management
    Position getPosition(const std::string& symbol);
    std::vector<Position> getAllPositions();
    void updatePositionPrices(const std::map<std::string, double>& current_prices);
    
    // Portfolio management
    Portfolio getPortfolio() const;
    double getAvailableCash() const { return portfolio.cash_balance; }
    double getTotalEquity() const { return portfolio.getEquity(); }
    
    // Risk management
    bool checkRiskLimits(const std::string& symbol, double quantity, double price);
    void setMaxPositionSize(double max_size) { max_position_size = max_size; }
    void setMaxDrawdown(double max_dd) { max_drawdown = max_dd; }
    
    // Trading signal processing
    void processTradingSignal(const TradingSignal& signal);
    
    // Performance metrics
    double getTotalPnL() const;
    double getDailyPnL() const;
    double getWinRate() const;
    double getSharpeRatio() const;
    
    // Paper trading specific
    void simulateFill(Order& order, double market_price);
    
private:
    TradingMode mode;
    Portfolio portfolio;
    std::shared_ptr<DatabaseManager> db_manager;
    
    // Risk parameters
    double max_position_size;
    double max_drawdown;
    double stop_loss_percentage;
    double take_profit_percentage;
    
    // Performance tracking
    double initial_balance;
    double peak_balance;
    std::vector<double> daily_returns;
    
    // Order management
    std::map<std::string, Order> pending_orders;
    std::map<std::string, Order> filled_orders;
    std::atomic<int> order_counter;
    std::mutex order_mutex;
    
    // Helper methods
    std::string generateOrderId();
    bool executeMarketOrder(Order& order, double market_price);
    bool executeLimitOrder(Order& order, double market_price);
    void updatePortfolio(const Order& order);
    double calculatePositionSize(const TradingSignal& signal);
    void applyStopLoss(Position& position, double current_price);
    void applyTakeProfit(Position& position, double current_price);
};

// Paper trading simulator
class PaperTradingSimulator {
public:
    PaperTradingSimulator();
    
    // Simulate market conditions
    double simulateSlippage(double price, double quantity, const std::string& side);
    double simulateSpread(double price);
    bool simulateOrderFill(const Order& order, double market_price);
    
    // Set simulation parameters
    void setSlippageRate(double rate) { slippage_rate = rate; }
    void setSpreadRate(double rate) { spread_rate = rate; }
    void setFillProbability(double prob) { fill_probability = prob; }
    
private:
    double slippage_rate;    // Percentage slippage
    double spread_rate;      // Bid-ask spread percentage
    double fill_probability; // Probability of limit order fill
};

} // namespace TradingSystem

#endif // TRADING_ENGINE_H