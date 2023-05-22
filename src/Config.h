#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>
#include <unistd.h>
#include <type_traits>

using ConfigMap = std::unordered_map<std::string, std::string>;

class Config {
public:
    bool begin(const char* fileName);

    const char* getEntry(const char* key, const char* defaultValue = nullptr) const;

    template <typename T>
    void setIfExists(const char* key, T* myValue) const {
        auto iterator = _config.find(key);
        if (iterator == _config.end()) {
            return;
        }
        set(myValue, iterator, std::is_integral<T>());
    }

private:
    const char* _broker = nullptr;
    const char* _caCert = nullptr;
    const char* _clientId = nullptr;
    int _port = 1883;
    const char* _topicTemplate = nullptr;
    int _keepalive = 60;
    ConfigMap _config;

    bool setDevice();

    template <typename T>
    void set(T* myValue, ConfigMap::const_iterator iterator, std::true_type) const {
        *myValue = static_cast<T>(std::stoi(iterator->second));
    }

    template <typename T>
    void set(T* myValue, ConfigMap::const_iterator iterator, std::false_type) const {
        *myValue = iterator->second;
    }
};

#endif
