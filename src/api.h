#ifndef API_H
#define API_H

#include <string>
#include <map>
#include <fstream>

std::map<std::string, std::string> loadEnv(const std::string& filename = ".env") { // parse our api key in our env
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
