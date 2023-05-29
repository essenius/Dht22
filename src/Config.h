#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>
#include <type_traits>
#include "OS.h"

using ConfigMap = std::unordered_map<std::string, std::string>;

class Config {
public:
    bool begin(const std::string& fileName, const std::string& hostName = "");

    std::string getEntry(const std::string& key, const std::string& defaultValue = "") const;

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
    void set(T* myValue, const ConfigMap::const_iterator iterator, std::false_type) const {
        *myValue = iterator->second;
    }
};

#endif
