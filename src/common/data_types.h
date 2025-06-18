#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <string>
#include <vector>
#include <chrono>

namespace TradingSystem {

struct MarketData {
    std::string symbol;
    double open;
    double high;
    double low;
    double close;
    double volume;
    std::chrono::system_clock::time_point timestamp;
    
    std::string toJson() const;
    static MarketData fromJson(const std::string& json);
};

struct TradingSignal {
    std::string symbol;
    double confidence;  // 0.0 to 1.0
    std::string action; // "BUY", "SELL", "HOLD"
    double suggested_position_size;
    std::chrono::system_clock::time_point timestamp;
    
    std::string toJson() const;
    static TradingSignal fromJson(const std::string& json);
};

struct Position {
    std::string symbol;
    double quantity;
    double entry_price;
    double current_price;
    double unrealized_pnl;
    std::chrono::system_clock::time_point entry_time;
    
    double getPnlPercentage() const {
        return ((current_price - entry_price) / entry_price) * 100.0;
    }
};

struct Order {
    std::string order_id;
    std::string symbol;
    std::string side; // "BUY" or "SELL"
    double quantity;
    double price;
    std::string order_type; // "MARKET", "LIMIT"
    std::string status; // "PENDING", "FILLED", "CANCELLED"
    std::chrono::system_clock::time_point timestamp;
};

} // namespace TradingSystem

#endif // DATA_TYPES_H