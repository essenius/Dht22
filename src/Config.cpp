// Copyright 2023 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#include <fstream>
#include <sstream>
#include <iostream>
#include "Config.h"

void Config::readStream(std::istream& inputStream) {
    std::string line;
    while (std::getline(inputStream, line)) {
        // ignore comments
        if (line.rfind('#', 0) == 0) {
            continue;
        }
        const size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string entity = line.substr(0, pos);
            const std::string value = line.substr(pos + 1);
            _config[entity] = value;
        }
    }
    if (_config.empty()) {
        std::cout << "Warning: did not find valid config data\n";
    }
}

bool Config::begin(const std::string& configInput, const std::string& hostName) {
    if (std::ifstream inputFile(configInput); !inputFile.is_open()) {
        std::stringstream stringStream(configInput);
        readStream(stringStream);
    } else {
        readStream(inputFile);
    }
    return setDevice(hostName);
}

bool Config::setDevice(std::string_view hostName) {
    if (const auto device = getEntry("device"); !device.empty()) return true;
    if (hostName.empty()) {
        std::cerr << "'device' not set in config, and failed to determine device name\n";
        return false;
    }
    _config["device"] = hostName;
    return true;
}

std::string Config::getEntry(const std::string& key, const std::string& defaultValue) const {
	const auto iterator = _config.find(key);
    if (iterator == _config.end()) {
        return defaultValue;
    }
    return iterator->second;
}
