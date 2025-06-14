#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>
#include "api.h"



class StockApi {
	public: 
		StockApi() {
			auto env = loadEnv();
			if (env.find("API_KEY") != env.end()) {
				m_ApiKey = env["API_KEY"];
				m_isInitialized = true;
			} else {
				std::cerr << "Warning: API_KEY not found in .env file" << std::endl;
				m_isInitialized = false;
			}
		}
		
		std::string getApiKey() const {
			return m_ApiKey;
		}
		
		bool isInitialized() const {
			return m_isInitialized;
		}
	
	private:
		std::string m_ApiKey;
		bool m_isInitialized = false;
};

int main() {
	StockApi api;
	
	if (api.isInitialized()) {
		std::cout << "API initialized successfully" << std::endl;
	} else {
		std::cerr << "Failed to initialize API" << std::endl;
		return 1;
	}
	
	return 0;
}
