#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>
#include "../common/data_types.h"

namespace TradingSystem {

class DatabaseManager {
public:
    DatabaseManager(const std::string& db_path = "trading_system.db");
    ~DatabaseManager();
    
    // Initialize database schema
    bool initialize();
    
    // Market data operations
    bool insertMarketData(const MarketData& data);
    std::vector<MarketData> getMarketData(const std::string& symbol, 
                                         int limit = 100);
    std::vector<MarketData> getMarketDataRange(const std::string& symbol,
                                              const std::chrono::system_clock::time_point& start,
                                              const std::chrono::system_clock::time_point& end);
    
    // Trading signals operations
    bool insertTradingSignal(const TradingSignal& signal);
    std::vector<TradingSignal> getLatestSignals(int limit = 10);
    
    // Position operations
    bool insertPosition(const Position& position);
    bool updatePosition(const Position& position);
    std::vector<Position> getOpenPositions();
    
    // Order operations
    bool insertOrder(const Order& order);
    bool updateOrderStatus(const std::string& order_id, const std::string& status);
    std::vector<Order> getPendingOrders();
    
    // Performance metrics
    double getTotalPnL();
    int getWinningTrades();
    int getLosingTrades();
    
private:
    sqlite3* db;
    std::string db_path;
    
    bool executeQuery(const std::string& query);
    bool createTables();
};

} // namespace TradingSystem

#endif // DATABASE_MANAGER_H