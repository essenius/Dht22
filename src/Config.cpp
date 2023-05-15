#include <fstream>
#include <iostream>
#include "Config.h"
#include <unistd.h>

bool Config::begin(const char* fileName) {
    std::ifstream input_file(fileName);
    if (!input_file.is_open()) {
        std::cerr << "Failed to open config file '" << fileName << "'\n";
        return false;
    }
    std::string line;
    while (std::getline(input_file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string entity = line.substr(0, pos);            
            std::string value = line.substr(pos + 1);
            _config[entity] = value;
        }
    }
    input_file.close();

    int size = gethostname(_hostName, _SC_HOST_NAME_MAX);
    printf("hostname=%s\n", _hostName);
    auto idTemplate = getEntry("idTemplate", nullptr);
    if (idTemplate == nullptr) {
        _config["id"] = _hostName;
    } else {
        auto id = std::string(idTemplate);
        auto locationOfString = id.find("%s");
        if (locationOfString != std::string::npos) {
            _hostName[size] = '\0';
            id.replace(locationOfString, 2, _hostName);
        };
        _config["id"] = id;
    }
    return true;
}

const char* Config::getEntry(const char* key, const char* defaultValue) const {
    auto value = _config.find(key);
    if (value == _config.end()) {
        return defaultValue;
    }
    return value->second.c_str();
}