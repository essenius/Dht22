#ifndef HOMIE_H
#define HOMIE_H

#include "Config.h"
#include "Mqtt.h"
#include "ISender.h"

class Homie : public ISender {
public:
    Homie(Queuing::Mqtt* mqtt, Config* config);
    virtual ~Homie();
    bool begin();
    bool sendHumidity(float value) override;
    bool sendMetadata();
    bool sendTemperature(float value) override;

private:
    static constexpr const char* HOMIE_PREFIX = "homie";
    static constexpr const char* HOMIE_VERSION = "4.0.0";
    static constexpr const char* TEMPERATURE =  "temperature";
    static constexpr const char* HUMIDITY = "humidity";
    static constexpr const char* NAME = "$name";

    bool sendMessage(const std::string& topic, const std::string& message, bool retain = true);
    void sendPropertyMetadata(const std::string& property, const std::string& unit);
    void sendState(const std::string& state);
    static std::string toString(float f);

    Queuing::Mqtt* _mqtt;
    Config* _config;
    bool _isConnected = false;
    std::string _deviceName;
    std::string _nodeName;
    std::string _prefix;
    std::string _nodePrefix;
    std::string _stateTopic;
};
#endif