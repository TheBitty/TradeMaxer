#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

namespace TradingSystem {

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }
    
    // Load configuration from file
    bool loadConfig(const std::string& config_file = "config.ini");
    
    // Get configuration values
    std::string getString(const std::string& section, const std::string& key, const std::string& default_value = "") const;
    int getInt(const std::string& section, const std::string& key, int default_value = 0) const;
    double getDouble(const std::string& section, const std::string& key, double default_value = 0.0) const;
    bool getBool(const std::string& section, const std::string& key, bool default_value = false) const;
    
    // Set configuration values
    void setString(const std::string& section, const std::string& key, const std::string& value);
    void setInt(const std::string& section, const std::string& key, int value);
    void setDouble(const std::string& section, const std::string& key, double value);
    void setBool(const std::string& section, const std::string& key, bool value);
    
    // Save configuration to file
    bool saveConfig(const std::string& config_file = "config.ini");
    
    // Load environment variables
    void loadEnvironment();
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    std::map<std::string, std::map<std::string, std::string>> config_data;
    
    std::string trim(const std::string& str) const;
    bool parseIniFile(const std::string& filename);
};

// Configuration structure for easy access
struct TradingConfig {
    // API settings
    std::string api_key;
    std::string api_base_url;
    int api_timeout_ms;
    
    // Trading settings
    std::string trading_mode;  // "paper" or "live"
    double initial_balance;
    double max_position_size;
    double max_drawdown;
    double stop_loss_percentage;
    double take_profit_percentage;
    
    // Database settings
    std::string db_path;
    
    // IPC settings
    std::string ipc_pipe_name;
    
    // Logging settings
    std::string log_level;
    std::string log_file;
    
    // Market data settings
    std::vector<std::string> symbols;
    int data_fetch_interval_seconds;
    int analysis_interval_seconds;
    
    // Load from ConfigManager
    static TradingConfig loadFromConfig();
};

} // namespace TradingSystem

#endif // CONFIG_MANAGER_H