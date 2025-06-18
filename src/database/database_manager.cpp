#include "database_manager.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace TradingSystem {

DatabaseManager::DatabaseManager(const std::string& db_path) : db(nullptr), db_path(db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        db = nullptr;
    }
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::initialize() {
    return createTables();
}

bool DatabaseManager::createTables() {
    std::vector<std::string> queries = {
        // Market data table
        "CREATE TABLE IF NOT EXISTS market_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "symbol TEXT NOT NULL,"
        "open REAL NOT NULL,"
        "high REAL NOT NULL,"
        "low REAL NOT NULL,"
        "close REAL NOT NULL,"
        "volume REAL NOT NULL,"
        "timestamp DATETIME NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        
        // Trading signals table
        "CREATE TABLE IF NOT EXISTS trading_signals ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "symbol TEXT NOT NULL,"
        "confidence REAL NOT NULL,"
        "action TEXT NOT NULL,"
        "suggested_position_size REAL NOT NULL,"
        "timestamp DATETIME NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        
        // Positions table
        "CREATE TABLE IF NOT EXISTS positions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "symbol TEXT NOT NULL,"
        "quantity REAL NOT NULL,"
        "entry_price REAL NOT NULL,"
        "current_price REAL,"
        "unrealized_pnl REAL,"
        "entry_time DATETIME NOT NULL,"
        "exit_time DATETIME,"
        "exit_price REAL,"
        "realized_pnl REAL,"
        "status TEXT DEFAULT 'OPEN',"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        
        // Orders table
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "order_id TEXT UNIQUE NOT NULL,"
        "symbol TEXT NOT NULL,"
        "side TEXT NOT NULL,"
        "quantity REAL NOT NULL,"
        "price REAL NOT NULL,"
        "order_type TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "timestamp DATETIME NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",
        
        // Create indexes for performance
        "CREATE INDEX IF NOT EXISTS idx_market_data_symbol_timestamp ON market_data(symbol, timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_trading_signals_timestamp ON trading_signals(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_positions_status ON positions(status);",
        "CREATE INDEX IF NOT EXISTS idx_orders_status ON orders(status);"
    };
    
    for (const auto& query : queries) {
        if (!executeQuery(query)) {
            return false;
        }
    }
    
    return true;
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool DatabaseManager::insertMarketData(const MarketData& data) {
    auto time_t = std::chrono::system_clock::to_time_t(data.timestamp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string query = "INSERT INTO market_data (symbol, open, high, low, close, volume, timestamp) VALUES ('"
        + data.symbol + "', "
        + std::to_string(data.open) + ", "
        + std::to_string(data.high) + ", "
        + std::to_string(data.low) + ", "
        + std::to_string(data.close) + ", "
        + std::to_string(data.volume) + ", '"
        + ss.str() + "');";
    
    return executeQuery(query);
}

std::vector<MarketData> DatabaseManager::getMarketData(const std::string& symbol, int limit) {
    std::vector<MarketData> results;
    sqlite3_stmt* stmt;
    
    std::string query = "SELECT symbol, open, high, low, close, volume, timestamp "
                       "FROM market_data WHERE symbol = ? "
                       "ORDER BY timestamp DESC LIMIT ?;";
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, limit);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MarketData data;
            data.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            data.open = sqlite3_column_double(stmt, 1);
            data.high = sqlite3_column_double(stmt, 2);
            data.low = sqlite3_column_double(stmt, 3);
            data.close = sqlite3_column_double(stmt, 4);
            data.volume = sqlite3_column_double(stmt, 5);
            
            // Parse timestamp
            std::string timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            std::tm tm = {};
            std::istringstream ss(timestamp_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            data.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            
            results.push_back(data);
        }
        
        sqlite3_finalize(stmt);
    }
    
    return results;
}

bool DatabaseManager::insertTradingSignal(const TradingSignal& signal) {
    auto time_t = std::chrono::system_clock::to_time_t(signal.timestamp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string query = "INSERT INTO trading_signals (symbol, confidence, action, suggested_position_size, timestamp) VALUES ('"
        + signal.symbol + "', "
        + std::to_string(signal.confidence) + ", '"
        + signal.action + "', "
        + std::to_string(signal.suggested_position_size) + ", '"
        + ss.str() + "');";
    
    return executeQuery(query);
}

std::vector<TradingSignal> DatabaseManager::getLatestSignals(int limit) {
    std::vector<TradingSignal> results;
    sqlite3_stmt* stmt;
    
    std::string query = "SELECT symbol, confidence, action, suggested_position_size, timestamp "
                       "FROM trading_signals "
                       "ORDER BY timestamp DESC LIMIT ?;";
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TradingSignal signal;
            signal.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            signal.confidence = sqlite3_column_double(stmt, 1);
            signal.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            signal.suggested_position_size = sqlite3_column_double(stmt, 3);
            
            // Parse timestamp
            std::string timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::tm tm = {};
            std::istringstream ss(timestamp_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            signal.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            
            results.push_back(signal);
        }
        
        sqlite3_finalize(stmt);
    }
    
    return results;
}

bool DatabaseManager::insertPosition(const Position& position) {
    auto time_t = std::chrono::system_clock::to_time_t(position.entry_time);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string query = "INSERT INTO positions (symbol, quantity, entry_price, current_price, unrealized_pnl, entry_time) VALUES ('"
        + position.symbol + "', "
        + std::to_string(position.quantity) + ", "
        + std::to_string(position.entry_price) + ", "
        + std::to_string(position.current_price) + ", "
        + std::to_string(position.unrealized_pnl) + ", '"
        + ss.str() + "');";
    
    return executeQuery(query);
}

std::vector<Position> DatabaseManager::getOpenPositions() {
    std::vector<Position> results;
    sqlite3_stmt* stmt;
    
    std::string query = "SELECT symbol, quantity, entry_price, current_price, unrealized_pnl, entry_time "
                       "FROM positions WHERE status = 'OPEN';";
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Position position;
            position.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            position.quantity = sqlite3_column_double(stmt, 1);
            position.entry_price = sqlite3_column_double(stmt, 2);
            position.current_price = sqlite3_column_double(stmt, 3);
            position.unrealized_pnl = sqlite3_column_double(stmt, 4);
            
            // Parse timestamp
            std::string timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            std::tm tm = {};
            std::istringstream ss(timestamp_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            position.entry_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            
            results.push_back(position);
        }
        
        sqlite3_finalize(stmt);
    }
    
    return results;
}

bool DatabaseManager::insertOrder(const Order& order) {
    auto time_t = std::chrono::system_clock::to_time_t(order.timestamp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::string query = "INSERT INTO orders (order_id, symbol, side, quantity, price, order_type, status, timestamp) VALUES ('"
        + order.order_id + "', '"
        + order.symbol + "', '"
        + order.side + "', "
        + std::to_string(order.quantity) + ", "
        + std::to_string(order.price) + ", '"
        + order.order_type + "', '"
        + order.status + "', '"
        + ss.str() + "');";
    
    return executeQuery(query);
}

bool DatabaseManager::updateOrderStatus(const std::string& order_id, const std::string& status) {
    std::string query = "UPDATE orders SET status = '" + status + "' WHERE order_id = '" + order_id + "';";
    return executeQuery(query);
}

double DatabaseManager::getTotalPnL() {
    double total_pnl = 0.0;
    sqlite3_stmt* stmt;
    
    std::string query = "SELECT SUM(realized_pnl) FROM positions WHERE status = 'CLOSED';";
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_pnl = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return total_pnl;
}

} // namespace TradingSystem