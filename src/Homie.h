#ifndef HOMIE_H
#define HOMIE_H

#include "SensorData.h"
#include "Config.h"
#include "Mqtt.h"

class Homie : public ISender {
public:
    Homie(Mqtt* mqtt, Config* config);
    ~Homie();
    bool begin();
    bool sendHumidity(float value) override;
    bool sendMetadata();
    bool sendTemperature(float value) override;
    void shutdown();

private:
    static constexpr const char* HOMIE_PREFIX = "homie";
    static constexpr const char* HOMIE_VERSION = "4.0.0";
    static constexpr const char* LOST = "lost";
    static constexpr const char* TEMPERATURE =  "temperature";
    static constexpr const char* HUMIDITY = "humidity";
    static constexpr const char* NAME = "$name";

    bool sendMessage(std::string topic, std::string message, bool retain = true);
    void sendPropertyMetadata(std::string property, std::string unit);
    void sendState(std::string state);
    std::string toString(float f);

    Mqtt* _mqtt;
    Config* _config;
    bool _isConnected = false;
    std::string _deviceName;
    std::string _nodeName;
    std::string _prefix;
    std::string _nodePrefix;
    std::string _stateTopic;
};
#endif