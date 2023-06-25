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

#include "Homie.h"
#include <iostream>

Homie::Homie(queuing::Mqtt *mqtt, Config* config): _mqtt(mqtt), _config(config) {}

Homie::~Homie() {
    printf("Homie destructor\n");
    if (_mqtt->isConnected()) {
        printf("Disconnecting from MQTT broker\n");
        sendState("disconnected");
    }
}

bool Homie::begin() {
    _deviceName = _config->getEntry("device");
    _nodeName = _config->getEntry("node", "climate");
    _prefix = std::string(HOMIE_PREFIX) + "/" + _deviceName + "/";
    _nodePrefix = _prefix + _nodeName + "/";
    _stateTopic = _prefix + "$state";
    _mqtt->setWill(_stateTopic);
    return _mqtt->begin();
}

std::string Homie::toString(const float f) {
    char buffer[20];
    (void)snprintf(buffer, sizeof(buffer), "%.1f", f);
    return { buffer };
}

bool Homie::sendTemperature(const float value) {
	const std::string topic = _nodePrefix + TEMPERATURE;
    return sendMessage(topic, toString(value), false);
}

bool Homie::sendHumidity(const float value) {
	const std::string topic = _nodePrefix + HUMIDITY;
    return sendMessage(topic, toString(value), false);
}

bool Homie::sendMessage(const std::string& topic, const std::string& message, const bool retain) {
    const bool isConnected = _mqtt->publish(topic, message, retain);
    std::cout << "MQTT publish t=" << topic << " m=" << message << " r=" << retain << " conn=" << isConnected << "_c=" << _isConnected << std::endl;
    if (isConnected != _isConnected) {
        _isConnected = isConnected;
        std::cout << "MQTT connected=" << _isConnected << std::endl;
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

void Homie::sendState(const std::string& state) {
    sendMessage(_stateTopic, state);
}

void Homie::sendPropertyMetadata(const std::string& property, const std::string& unit) {
    sendMessage(_nodePrefix + property + "/" + NAME, property);
    sendMessage(_nodePrefix + property + "/$datatype", "float");
    sendMessage(_nodePrefix + property + "/$unit", unit);
    sendMessage(_nodePrefix + property + "/$settable", "false");
}
