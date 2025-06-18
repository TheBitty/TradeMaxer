#include "data_types.h"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace TradingSystem {

std::string MarketData::toJson() const {
    std::stringstream ss;
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    ss << "{"
       << "\"symbol\":\"" << symbol << "\","
       << "\"open\":" << std::fixed << std::setprecision(2) << open << ","
       << "\"high\":" << high << ","
       << "\"low\":" << low << ","
       << "\"close\":" << close << ","
       << "\"volume\":" << volume << ","
       << "\"timestamp\":\"" << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\""
       << "}";
    
    return ss.str();
}

MarketData MarketData::fromJson(const std::string& json) {
    MarketData data;
    // Simple JSON parsing - in production, use a proper JSON library
    
    size_t pos = json.find("\"symbol\":\"");
    if (pos != std::string::npos) {
        pos += 10;
        size_t end = json.find("\"", pos);
        data.symbol = json.substr(pos, end - pos);
    }
    
    pos = json.find("\"open\":");
    if (pos != std::string::npos) {
        pos += 7;
        data.open = std::stod(json.substr(pos));
    }
    
    pos = json.find("\"high\":");
    if (pos != std::string::npos) {
        pos += 7;
        data.high = std::stod(json.substr(pos));
    }
    
    pos = json.find("\"low\":");
    if (pos != std::string::npos) {
        pos += 6;
        data.low = std::stod(json.substr(pos));
    }
    
    pos = json.find("\"close\":");
    if (pos != std::string::npos) {
        pos += 8;
        data.close = std::stod(json.substr(pos));
    }
    
    pos = json.find("\"volume\":");
    if (pos != std::string::npos) {
        pos += 9;
        data.volume = std::stod(json.substr(pos));
    }
    
    data.timestamp = std::chrono::system_clock::now();
    
    return data;
}

std::string TradingSignal::toJson() const {
    std::stringstream ss;
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    ss << "{"
       << "\"symbol\":\"" << symbol << "\","
       << "\"confidence\":" << std::fixed << std::setprecision(4) << confidence << ","
       << "\"action\":\"" << action << "\","
       << "\"suggested_position_size\":" << suggested_position_size << ","
       << "\"timestamp\":\"" << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\""
       << "}";
    
    return ss.str();
}

TradingSignal TradingSignal::fromJson(const std::string& json) {
    TradingSignal signal;
    
    size_t pos = json.find("\"symbol\":\"");
    if (pos != std::string::npos) {
        pos += 10;
        size_t end = json.find("\"", pos);
        signal.symbol = json.substr(pos, end - pos);
    }
    
    pos = json.find("\"confidence\":");
    if (pos != std::string::npos) {
        pos += 13;
        signal.confidence = std::stod(json.substr(pos));
    }
    
    pos = json.find("\"action\":\"");
    if (pos != std::string::npos) {
        pos += 10;
        size_t end = json.find("\"", pos);
        signal.action = json.substr(pos, end - pos);
    }
    
    pos = json.find("\"suggested_position_size\":");
    if (pos != std::string::npos) {
        pos += 26;
        signal.suggested_position_size = std::stod(json.substr(pos));
    }
    
    signal.timestamp = std::chrono::system_clock::now();
    
    return signal;
}

} // namespace TradingSystem