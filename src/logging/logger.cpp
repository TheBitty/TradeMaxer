#include "logger.h"
#include <algorithm>

namespace TradingSystem {

void Logger::initialize(const std::string& log_file_path, LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    
    // Close existing file if open
    if (log_file.is_open()) {
        log_file.close();
    }
    
    // Open new log file
    log_file.open(log_file_path, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << log_file_path << std::endl;
        console_output = true; // Force console output if file fails
    }
    
    log_level = level;
    
    // Write initialization message
    writeLog("INFO", "Logger initialized - Level: " + levelToString(level));
}

void Logger::setLogLevel(const std::string& level) {
    log_level = stringToLevel(level);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level >= log_level) {
        writeLog(levelToString(level), message);
    }
}

void Logger::logPerformance(const std::string& operation, double duration_ms) {
    std::stringstream ss;
    ss << "Performance - " << operation << ": " 
       << std::fixed << std::setprecision(2) << duration_ms << " ms";
    log(LogLevel::DEBUG, ss.str());
}

void Logger::logTrade(const std::string& symbol, const std::string& side, 
                     double quantity, double price) {
    std::stringstream ss;
    ss << "TRADE - " << side << " " << quantity << " " << symbol 
       << " @ $" << std::fixed << std::setprecision(2) << price;
    log(LogLevel::INFO, ss.str());
}

void Logger::logSignal(const std::string& symbol, const std::string& action, 
                      double confidence) {
    std::stringstream ss;
    ss << "SIGNAL - " << symbol << ": " << action 
       << " (confidence: " << std::fixed << std::setprecision(2) 
       << confidence * 100 << "%)";
    log(LogLevel::INFO, ss.str());
}

void Logger::logPosition(const std::string& symbol, double quantity, 
                        double entry_price, double current_pnl) {
    std::stringstream ss;
    ss << "POSITION - " << symbol << ": " << quantity << " units @ $" 
       << std::fixed << std::setprecision(2) << entry_price
       << ", P&L: $" << current_pnl;
    log(LogLevel::INFO, ss.str());
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) {
        log_file.flush();
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
             (now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& level) {
    std::string upper_level = level;
    std::transform(upper_level.begin(), upper_level.end(), 
                  upper_level.begin(), ::toupper);
    
    if (upper_level == "DEBUG") return LogLevel::DEBUG;
    if (upper_level == "INFO") return LogLevel::INFO;
    if (upper_level == "WARNING") return LogLevel::WARNING;
    if (upper_level == "ERROR") return LogLevel::ERROR;
    if (upper_level == "CRITICAL") return LogLevel::CRITICAL;
    
    return LogLevel::INFO; // Default
}

void Logger::writeLog(const std::string& level_str, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::stringstream log_line;
    log_line << "[" << timestamp << "] [" << level_str << "] " << message;
    
    // Write to file if available
    if (log_file.is_open()) {
        log_file << log_line.str() << std::endl;
        log_file.flush(); // Ensure immediate write
    }
    
    // Also write to console if enabled or file is not available
    if (console_output || !log_file.is_open()) {
        if (level_str == "ERROR" || level_str == "CRITICAL") {
            std::cerr << log_line.str() << std::endl;
        } else {
            std::cout << log_line.str() << std::endl;
        }
    }
}

} // namespace TradingSystem