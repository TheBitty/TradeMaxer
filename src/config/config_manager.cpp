#include "config_manager.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>

namespace TradingSystem {

bool ConfigManager::loadConfig(const std::string& config_file) {
    // First load environment variables
    loadEnvironment();
    
    // Then load from config file
    return parseIniFile(config_file);
}

bool ConfigManager::parseIniFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Config file " << filename << " not found" << std::endl;
        return false;
    }
    
    std::string line;
    std::string current_section = "default";
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Check for section header
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key-value pair
        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = trim(line.substr(0, eq_pos));
            std::string value = trim(line.substr(eq_pos + 1));
            
            config_data[current_section][key] = value;
        }
    }
    
    file.close();
    return true;
}

void ConfigManager::loadEnvironment() {
    // Load common environment variables
    const char* api_key = std::getenv("API_KEY");
    if (api_key) {
        config_data["api"]["key"] = api_key;
    }
    
    const char* trading_mode = std::getenv("TRADING_MODE");
    if (trading_mode) {
        config_data["trading"]["mode"] = trading_mode;
    }
    
    const char* log_level = std::getenv("LOG_LEVEL");
    if (log_level) {
        config_data["logging"]["level"] = log_level;
    }
}

std::string ConfigManager::getString(const std::string& section, const std::string& key, const std::string& default_value) const {
    auto section_it = config_data.find(section);
    if (section_it != config_data.end()) {
        auto key_it = section_it->second.find(key);
        if (key_it != section_it->second.end()) {
            return key_it->second;
        }
    }
    return default_value;
}

int ConfigManager::getInt(const std::string& section, const std::string& key, int default_value) const {
    std::string value = getString(section, key, "");
    if (!value.empty()) {
        try {
            return std::stoi(value);
        } catch (...) {
            std::cerr << "Invalid integer value for " << section << "." << key << std::endl;
        }
    }
    return default_value;
}

double ConfigManager::getDouble(const std::string& section, const std::string& key, double default_value) const {
    std::string value = getString(section, key, "");
    if (!value.empty()) {
        try {
            return std::stod(value);
        } catch (...) {
            std::cerr << "Invalid double value for " << section << "." << key << std::endl;
        }
    }
    return default_value;
}

bool ConfigManager::getBool(const std::string& section, const std::string& key, bool default_value) const {
    std::string value = getString(section, key, "");
    if (!value.empty()) {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return (value == "true" || value == "1" || value == "yes" || value == "on");
    }
    return default_value;
}

void ConfigManager::setString(const std::string& section, const std::string& key, const std::string& value) {
    config_data[section][key] = value;
}

void ConfigManager::setInt(const std::string& section, const std::string& key, int value) {
    config_data[section][key] = std::to_string(value);
}

void ConfigManager::setDouble(const std::string& section, const std::string& key, double value) {
    config_data[section][key] = std::to_string(value);
}

void ConfigManager::setBool(const std::string& section, const std::string& key, bool value) {
    config_data[section][key] = value ? "true" : "false";
}

bool ConfigManager::saveConfig(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << config_file << std::endl;
        return false;
    }
    
    for (const auto& [section, keys] : config_data) {
        file << "[" << section << "]" << std::endl;
        for (const auto& [key, value] : keys) {
            file << key << " = " << value << std::endl;
        }
        file << std::endl;
    }
    
    file.close();
    return true;
}

std::string ConfigManager::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// TradingConfig implementation
TradingConfig TradingConfig::loadFromConfig() {
    TradingConfig config;
    ConfigManager& cm = ConfigManager::getInstance();
    
    // API settings
    config.api_key = cm.getString("api", "key", "");
    config.api_base_url = cm.getString("api", "base_url", "https://api.polygon.io");
    config.api_timeout_ms = cm.getInt("api", "timeout_ms", 5000);
    
    // Trading settings
    config.trading_mode = cm.getString("trading", "mode", "paper");
    config.initial_balance = cm.getDouble("trading", "initial_balance", 10000.0);
    config.max_position_size = cm.getDouble("trading", "max_position_size", 5000.0);
    config.max_drawdown = cm.getDouble("trading", "max_drawdown", 0.20);
    config.stop_loss_percentage = cm.getDouble("trading", "stop_loss_percentage", 0.02);
    config.take_profit_percentage = cm.getDouble("trading", "take_profit_percentage", 0.05);
    
    // Database settings
    config.db_path = cm.getString("database", "path", "trading_system.db");
    
    // IPC settings
    config.ipc_pipe_name = cm.getString("ipc", "pipe_name", "/tmp/trading_system_pipe");
    
    // Logging settings
    config.log_level = cm.getString("logging", "level", "INFO");
    config.log_file = cm.getString("logging", "file", "trading_system.log");
    
    // Market data settings
    std::string symbols_str = cm.getString("market_data", "symbols", "BTC,ETH,DOGE");
    std::stringstream ss(symbols_str);
    std::string symbol;
    while (std::getline(ss, symbol, ',')) {
        config.symbols.push_back(symbol);
    }
    
    config.data_fetch_interval_seconds = cm.getInt("market_data", "fetch_interval", 60);
    config.analysis_interval_seconds = cm.getInt("market_data", "analysis_interval", 300);
    
    return config;
}

} // namespace TradingSystem