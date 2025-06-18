#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <memory>

namespace TradingSystem {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    // Initialize logger
    void initialize(const std::string& log_file = "trading_system.log", 
                   LogLevel level = LogLevel::INFO);
    
    // Set log level
    void setLogLevel(LogLevel level) { log_level = level; }
    void setLogLevel(const std::string& level);
    
    // Logging methods
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    // Generic log method
    void log(LogLevel level, const std::string& message);
    
    // Performance logging
    void logPerformance(const std::string& operation, double duration_ms);
    
    // Trading specific logging
    void logTrade(const std::string& symbol, const std::string& side, 
                  double quantity, double price);
    void logSignal(const std::string& symbol, const std::string& action, 
                   double confidence);
    void logPosition(const std::string& symbol, double quantity, 
                    double entry_price, double current_pnl);
    
    // Flush log buffer
    void flush();
    
private:
    Logger() : log_level(LogLevel::INFO), console_output(true) {}
    ~Logger() { 
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::ofstream log_file;
    LogLevel log_level;
    bool console_output;
    std::mutex log_mutex;
    
    std::string getCurrentTimestamp();
    std::string levelToString(LogLevel level);
    LogLevel stringToLevel(const std::string& level);
    void writeLog(const std::string& level_str, const std::string& message);
};

// Convenience macros
#define LOG_DEBUG(msg) TradingSystem::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) TradingSystem::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) TradingSystem::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) TradingSystem::Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) TradingSystem::Logger::getInstance().critical(msg)

// Performance timer helper
class PerformanceTimer {
public:
    PerformanceTimer(const std::string& operation_name) 
        : operation(operation_name), 
          start_time(std::chrono::high_resolution_clock::now()) {}
    
    ~PerformanceTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>
                       (end_time - start_time).count();
        Logger::getInstance().logPerformance(operation, duration / 1000.0);
    }
    
private:
    std::string operation;
    std::chrono::high_resolution_clock::time_point start_time;
};

} // namespace TradingSystem

#endif // LOGGER_H