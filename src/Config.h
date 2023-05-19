#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <string>
#include <unistd.h>

using ConfigMap = std::unordered_map<std::string, std::string>;

class Config {
public:
    bool begin(const char* fileName);

    const char* getEntry(const char* key, const char* defaultValue = nullptr) const;

    template <typename T>
    void setIfExists(const char* key, T* value) const {
        auto iterator = _config.find(key);
        if (iterator == _config.end()) {
            return;
        }
        *value = static_cast<T>(std::stoi(iterator->second));
    }

private:
    char _hostName[_SC_HOST_NAME_MAX];
    const char* _broker = nullptr;
    const char* _caCert = nullptr;
    const char* _clientId = nullptr;
    int _port = 1883;
    const char* _topicTemplate = nullptr;
    int _keepalive = 60;
    ConfigMap _config;
    
    bool setId();

};

#endif
