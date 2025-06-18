#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "api.h"


int main() {
	StockApi api;
	
	if (api.isInitialized()) {
		std::cout << "API initialized successfully" << std::endl;
		
		// Fetch and display BTC price data from Polygon API
		try {
			std::cout << "Fetching BTC price..." << std::endl;
			CryptoPrice btc = api.getBTCPrice();
			
			// Display formatted BTC price information
			std::cout << "\n=== BTC Price Information ===" << std::endl;
			std::cout << "Symbol: " << btc.symbol << std::endl;
			std::cout << "Current Price: $" << std::fixed << std::setprecision(2) << btc.price << std::endl;
			std::cout << "Open: $" << btc.open << std::endl;
			std::cout << "High: $" << btc.high << std::endl;
			std::cout << "Low: $" << btc.low << std::endl;
			std::cout << "Volume: " << btc.volume << std::endl;
			std::cout << "Date: " << btc.date << std::endl;
			
		} catch (const std::exception& e) {
			std::cerr << "Error fetching BTC price: " << e.what() << std::endl;
		}
		
		std::cout << "\nLaunching Python neural network..." << std::endl;
		int result = system("python3 stock_ranking_nn.py");
		
		if (result == 0) {
			std::cout << "Python script completed successfully" << std::endl;
		} else {
			std::cerr << "Python script failed with code: " << result << std::endl;
		}
	} else {
		std::cerr << "Failed to initialize API" << std::endl;
		return 1;
	}
	
	return 0;
}
