#include "Homie.h"
#include <cstring>
#include <iostream>

Homie::Homie(Mqtt *mqtt, Config* config): _mqtt(mqtt), _config(config) {}

Homie::~Homie() {
    printf("Homie destructor\n");
    shutdown();
}

bool Homie::begin() {
    _deviceName = _config->getEntry("device");
    _nodeName = _config->getEntry("node", "climate");
    _prefix = std::string(HOMIE_PREFIX) + "/" + _deviceName + "/";
    _nodePrefix = _prefix + _nodeName + "/";
    _stateTopic = _prefix + "$state";

    _mqtt->will_set(_stateTopic.c_str(), strlen(LOST), LOST);
    return _mqtt->begin();
}

std::string Homie::toString(float f) {
    char buffer[20];
    (void)snprintf(buffer, sizeof(buffer), "%.1f", f);
    return { buffer };
}

bool Homie::sendTemperature(float value) {
	const std::string topic = _nodePrefix + TEMPERATURE;
    return sendMessage(topic, toString(value), false);
}

bool Homie::sendHumidity(float value) {
	const std::string topic = _nodePrefix + HUMIDITY;
    return sendMessage(topic, toString(value), false);
}

bool Homie::sendMessage(const std::string& topic, const std::string& message, bool retain) {
    int returnValue = _mqtt->publish(nullptr, topic.c_str(), static_cast<int>(message.length()), message.c_str(), 0, retain);
    bool isConnected = returnValue == MOSQ_ERR_SUCCESS;
    if (isConnected != _isConnected) {
        _isConnected = isConnected;
        if (isConnected) sendState("ready");
    } 
    return isConnected;
}

bool Homie::sendMetadata() {
    if (!sendMessage(_prefix + "$homie", HOMIE_VERSION)) return false;
    // assume that next sendMessage calls succeed if the first one does
    sendMessage(_prefix + NAME, _deviceName);
    sendMessage(_prefix + "$nodes", _nodeName);
    sendMessage(_prefix + "$extensions", "");
    sendMessage(_prefix + "$implementation", "pi-zero-w");
    sendMessage(_nodePrefix + NAME, _nodeName);
    sendMessage(_nodePrefix + "$type", "climate");
    sendMessage(_nodePrefix + "$properties", std::string(TEMPERATURE) + "," + HUMIDITY);
    sendPropertyMetadata(TEMPERATURE, "°C");
    sendPropertyMetadata(HUMIDITY, "%");
    return true;
}

void Homie::shutdown() {
    printf("Shutdown Homie\n");
    if (_mqtt->isConnected()) {
        printf("Disconnecting from MQTT broker\n");
        sendState("disconnected");
    }
    _mqtt->shutdown();
}

void Homie::sendState(const std::string& state) {
    sendMessage(_stateTopic, state);
}

void Homie::sendPropertyMetadata(const std::string& property, const std::string& unit) {
    sendMessage(_nodePrefix + property + "/" + NAME, property);
    sendMessage(_nodePrefix + property + "/$datatype", "float");
    sendMessage(_nodePrefix + property + "/$unit", unit);
    sendMessage(_nodePrefix + property + "/$settable", "false");
}
