#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>
#include <type_traits>

using ConfigMap = std::unordered_map<std::string, std::string>;

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

class Config {
public:
    bool begin(const std::string& configInput, const std::string& hostName = "");

    [[nodiscard]] std::string getEntry(const std::string& key, const std::string& defaultValue = "") const;

    template <typename T>
    void setIfExists(const std::string& key, T* myValue) const {
        auto iterator = _config.find(key);
        if (iterator == _config.end()) {
            return;
        }
        set(myValue, iterator, std::is_integral<T>());
    }

private:
    ConfigMap _config{};

    void readStream(std::istream & inputStream);
    bool setDevice(std::string_view hostName);

    template <typename T>
    void set(T* myValue, const ConfigMap::const_iterator iterator, std::true_type) const {
        *myValue = static_cast<T>(std::stoi(iterator->second));
    }

    template <typename T>
    static void set(T* myValue, const ConfigMap::const_iterator iterator, std::false_type) {
        *myValue = iterator->second;
    }
};

#endif
