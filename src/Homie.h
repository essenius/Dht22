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

#ifndef HOMIE_H
#define HOMIE_H

#include "Config.h"
#include "Mqtt.h"
#include "ISender.h"

class Homie final : public ISender {
public:
    Homie(queuing::Mqtt* mqtt, Config* config);
    ~Homie() override;
    Homie(const Homie&) = delete;
    Homie(Homie&&) = delete;
    Homie& operator=(const Homie&) = delete;
    ISender& operator=(Homie&&) = delete;
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

    queuing::Mqtt* _mqtt;
    Config* _config;
    bool _isConnected = false;
    std::string _deviceName;
    std::string _nodeName;
    std::string _prefix;
    std::string _nodePrefix;
    std::string _stateTopic;
};
#endif