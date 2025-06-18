#ifndef API_H
#define API_H

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <curl/curl.h>

// Forward declaration
std::map<std::string, std::string> loadEnv(const std::string& filename = ".env");

// Structure to hold cryptocurrency price data from Polygon API
struct CryptoPrice {
	std::string symbol;    // Cryptocurrency symbol (e.g., "BTC")
	double price;          // Current/close price
	double open;           // Opening price
	double high;           // Highest price in period
	double low;            // Lowest price in period
	double volume;         // Trading volume
	std::string date;      // Date/time of data
};

// Structure to hold HTTP response data from curl
struct WriteResponse {
	std::string data;
};

// Callback function for curl to write HTTP response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, WriteResponse* response) {
	size_t totalSize = size * nmemb;
	response->data.append((char*)contents, totalSize);
	return totalSize;
}

class StockApi {
	
	public: 
		// Constructor: Initialize API with key from .env file and setup curl
		StockApi() {
			auto env = loadEnv(".env");
			if (env.find("API_KEY") != env.end()) {
				m_ApiKey = env["API_KEY"];
				m_isInitialized = true;
			} else {
				std::cerr << "Warning: API_KEY not found in .env file" << std::endl;
				m_isInitialized = false;
			}
			curl_global_init(CURL_GLOBAL_DEFAULT);
		}
		
		// Destructor: Cleanup curl resources
		~StockApi() {
			curl_global_cleanup();
		}
		
		// Get the loaded API key
		std::string getApiKey() const {
			return m_ApiKey;
		}
		
		// Check if API was properly initialized with valid key
		bool isInitialized() const {
			return m_isInitialized;
		}
		
		// Fetch BTC price data from Polygon API
		// Uses the aggregates endpoint to get previous day's OHLCV data
		CryptoPrice getBTCPrice() {
			if (!m_isInitialized) {
				throw std::runtime_error("API not initialized");
			}
			
			CURL* curl = curl_easy_init();
			CryptoPrice result;
			
			if (curl) {
				// Build Polygon API URL for BTC/USD previous day data
				std::string url = "https://api.polygon.io/v2/aggs/ticker/X:BTCUSD/prev?apikey=" + m_ApiKey;
				WriteResponse response;
				
				// Configure curl options
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
				curl_easy_setopt(curl, CURLOPT_USERAGENT, "TradingSystem/1.0");
				
				// Execute HTTP request
				CURLcode res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);
				
				if (res == CURLE_OK) {
					// Simple JSON parsing without external library
					std::string jsonStr = response.data;
					
					// Check if response contains success status
					if (jsonStr.find("\"status\":\"OK\"") == std::string::npos) {
						throw std::runtime_error("API returned error status");
					}
					
					// Extract values using string parsing (basic implementation)
					double close_price = 0.0, open_price = 0.0, high_price = 0.0, low_price = 0.0, volume = 0.0;
					
					// Find the results array and extract first result
					size_t results_pos = jsonStr.find("\"results\":[{");
					if (results_pos != std::string::npos) {
						std::string results_section = jsonStr.substr(results_pos);
						
						// Extract close price
						size_t c_pos = results_section.find("\"c\":");
						if (c_pos != std::string::npos) {
							size_t start = c_pos + 4;
							size_t end = results_section.find_first_of(",}", start);
							if (end != std::string::npos) {
								close_price = std::stod(results_section.substr(start, end - start));
							}
						}
						
						// Extract open price
						size_t o_pos = results_section.find("\"o\":");
						if (o_pos != std::string::npos) {
							size_t start = o_pos + 4;
							size_t end = results_section.find_first_of(",}", start);
							if (end != std::string::npos) {
								open_price = std::stod(results_section.substr(start, end - start));
							}
						}
						
						// Extract high price
						size_t h_pos = results_section.find("\"h\":");
						if (h_pos != std::string::npos) {
							size_t start = h_pos + 4;
							size_t end = results_section.find_first_of(",}", start);
							if (end != std::string::npos) {
								high_price = std::stod(results_section.substr(start, end - start));
							}
						}
						
						// Extract low price
						size_t l_pos = results_section.find("\"l\":");
						if (l_pos != std::string::npos) {
							size_t start = l_pos + 4;
							size_t end = results_section.find_first_of(",}", start);
							if (end != std::string::npos) {
								low_price = std::stod(results_section.substr(start, end - start));
							}
						}
						
						// Extract volume
						size_t v_pos = results_section.find("\"v\":");
						if (v_pos != std::string::npos) {
							size_t start = v_pos + 4;
							size_t end = results_section.find_first_of(",}", start);
							if (end != std::string::npos) {
								volume = std::stod(results_section.substr(start, end - start));
							}
						}
					}
					
					// Set extracted values
					result.symbol = "BTC";
					result.price = close_price;
					result.open = open_price;
					result.high = high_price;
					result.low = low_price;
					result.volume = volume;
					result.date = "Latest";
					
					if (close_price == 0.0) {
						throw std::runtime_error("Failed to parse price data from API response");
					}
				} else {
					throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
				}
			} else {
				throw std::runtime_error("Failed to initialize CURL");
			}
			
			return result;
		}
	
	private:
		std::string m_ApiKey;        // Polygon API key from .env file
		bool m_isInitialized = false; // Flag indicating successful initialization
};

// Function to load environment variables from .env file
std::map<std::string, std::string> loadEnv(const std::string& filename) {
    std::map<std::string, std::string> env;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            env[key] = value;
        }
    }
    
    return env;
}

#endif // API_H
