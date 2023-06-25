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

#ifndef MQTT1_H
#define MQTT1_H

#include <mosquitto.h>
#include "Config.h"

namespace queuing {
    void onConnect(mosquitto* mosquittoInstance, void* userdata, int returnCode);
    void onDisconnect(mosquitto* mosquittoInstance, void* userdata, int returnCode);
    void onPublish(mosquitto* mosquittoInstance, void* userdata, int messageId);
    void onLog(mosquitto* mosquittoInstance, void* userdata, int level, const char* str);

    class Mqtt {
    public:
        explicit Mqtt(const Config* config, volatile bool* keepGoing);
        ~Mqtt();
        bool begin();
        int errorCode() const { return _errorCode; }
        bool isConnected() const { return _isConnected; }
        bool publish(const std::string& topic, const std::string& message, bool retain = false);
        void setWill(const std::string& topic) const;
        bool verifyConnection() const;
        bool waitForConnection() const;
        friend void onConnect(mosquitto* mosquittoInstance, void* userdata, int returnCode);
        friend void onDisconnect(mosquitto* mosquittoInstance, void* userdata, int returnCode);
        friend void onPublish(mosquitto* mosquittoInstance, void* userdata, int messageId);
        friend void onLog(mosquitto* mosquittoInstance, void* userdata, int level, const char* str);


    private:
        const Config* _config;
        mosquitto* _mosquitto;
        const char* _broker = nullptr;
        const char* _caCert = nullptr;
        const char* _user = nullptr;
        const char* _password = nullptr;
        int _errorCode = 0;
        int _port = 1883;
        int _keepAliveSeconds = 60;
        bool _isConnected = false;
        volatile bool* _keepGoing = nullptr;
        bool firstConnect();
        void setConnected(bool connected) { _isConnected = connected; }
        void setErrorCode(int returnCode) { _errorCode = returnCode; }
    };
}

#endif