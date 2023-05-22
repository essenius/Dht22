#include <fstream>
#include <iostream>
#include "Config.h"
#include <unistd.h>
#include <cstring>
#include <algorithm>

bool Config::begin(const char* fileName) {
    std::ifstream input_file(fileName);
    if (!input_file.is_open()) {
        std::cerr << "Failed to open config file '" << fileName << "'\n";
        return false;
    }
    std::string line;
    while (std::getline(input_file, line)) {
        // ignore comments
        if (line.rfind("#", 0) == 0) {
            continue;
        }
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string entity = line.substr(0, pos);            
            std::string value = line.substr(pos + 1);
            _config[entity] = value;
        }
    }
    input_file.close();
    return setDevice();
}

bool Config::setDevice() {
    if (auto device = getEntry("device"); device != nullptr) return true;
    printf("setting device to host name\n");
    char hostName[_SC_HOST_NAME_MAX];
    if (gethostname(hostName, _SC_HOST_NAME_MAX) != 0) {
        std::cerr << "Failed to get device name\n";
        return false;
    }
    std::transform(hostName, hostName + strlen(hostName), hostName, ::tolower);
    _config["device"] = hostName;
    return true;
}

const char* Config::getEntry(const char* key, const char* defaultValue) const {
    auto iterator = _config.find(key);
    if (iterator == _config.end()) {
        return defaultValue;
    }
    return iterator->second.c_str();
}
