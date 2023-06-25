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

#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>

#include "Mqtt.h"
#include <mosquitto.h>

namespace queuing {
    void onConnect(mosquitto *mosquittoInstance, void *userdata, int returnCode) {
        (void)mosquittoInstance;
        const auto mqtt = static_cast<Mqtt*>(userdata);
        mqtt->setErrorCode(returnCode);
        mqtt->setConnected(returnCode == MOSQ_ERR_SUCCESS);
        if (mqtt->isConnected()) {
            std::cout << "## Connected" << std::endl;
        }
        else {
            std::cout << "## Failed connecting - code " << returnCode << ": " << mosquitto_strerror(returnCode) << std::endl;
        }
    }

    void onDisconnect(mosquitto *mosquittoInstance, void *userdata, int returnCode) {
        (void)mosquittoInstance;
        const auto mqtt = static_cast<Mqtt*>(userdata);
        mqtt->setErrorCode(returnCode);
        mqtt->setConnected(false);
        std::cout << "## Disconnected" << std::endl;
    }

    void onPublish(mosquitto *mosquittoInstance, void *userdata, int messageId) {
        (void)mosquittoInstance;
        (void)userdata;
        std::cout << "## - Message published successfully: " << messageId << std::endl;
    }

    void onLog(struct mosquitto *mosquittoInstance, void *userdata, const int level, const char *str) {
        (void)mosquittoInstance;
        (void)userdata;
        std::cout << "## - Log: " << level << ": " << str << std::endl;
    }

    Mqtt::Mqtt(const Config* config, volatile bool* keepGoing) : _config(config), _keepGoing(keepGoing) {
	    const auto id = config->getEntry("device");
        mosquitto_lib_init();
        _mosquitto = mosquitto_new(id.c_str(), true, this);
        mosquitto_connect_callback_set(_mosquitto, &onConnect);
        mosquitto_disconnect_callback_set(_mosquitto, &onDisconnect);
        mosquitto_publish_callback_set(_mosquitto, &onPublish);
        mosquitto_log_callback_set(_mosquitto, &onLog);  
    }

    bool Mqtt::begin() {
        _caCert = _config->getEntry("caCert").c_str();
        _broker = _config->getEntry("broker", "localhost").c_str();
        _config->setIfExists("port", &_port);
        _user = _config->getEntry("user").c_str();
        _password = _config->getEntry("password").c_str();
        _config->setIfExists("keepAliveSeconds", &_keepAliveSeconds);

        return firstConnect();
    }

    bool Mqtt::firstConnect() {
        if (strlen(_caCert) > 0) {
            printf("setting ca cert %s\n", _caCert);
            if (mosquitto_tls_set(_mosquitto, _caCert, nullptr, nullptr, nullptr, nullptr) != MOSQ_ERR_SUCCESS) {
                std::cerr << "failed\n";
                return false;
            }
        }
        if (strlen(_user) > 0) {
            printf("setting user %s\n", _user);
            if (mosquitto_username_pw_set(_mosquitto, _user, _password) != MOSQ_ERR_SUCCESS) {
                std::cerr << "failed\n";
                return false;
            }
        }

        printf("Connecting to %s:%d, with keep-alive %d\n", _broker, _port, _keepAliveSeconds);
        if (const int rc = mosquitto_connect(_mosquitto, _broker, _port, _keepAliveSeconds); rc != MOSQ_ERR_SUCCESS) {
            _errorCode = rc;
            std::cerr << "Connect failed, error: " << rc << "/" << mosquitto_strerror(rc) << "\n";
            return false;
        }

        printf("starting loop\n");
        mosquitto_loop_start(_mosquitto);        
        std::cout << "Started loop\n";
        return true;
    }

    Mqtt::~Mqtt() {
        std::cout << "##-Mqtt Destructor-##" << std::endl; 
        mosquitto_disconnect(_mosquitto);
        mosquitto_loop_stop(_mosquitto, true);
        mosquitto_destroy(_mosquitto);
        mosquitto_lib_cleanup();
    }

    bool Mqtt::publish(const std::string& topic, const std::string& message, bool retain) {
        if (!verifyConnection()) return false;
        int messageId;
        _errorCode = mosquitto_publish(_mosquitto, &messageId, topic.c_str(), 
                            static_cast<int>(message.length()), message.c_str(), 0, retain);

        if (_errorCode != MOSQ_ERR_SUCCESS) {
            std::cerr << "Publish failed, error: " << _errorCode << "/" << mosquitto_strerror(_errorCode) << "\n";
            return false;
        }
        return true;
    }

    void Mqtt::setWill(const std::string &topic) const {
        constexpr const char* LOST = "lost";
        mosquitto_will_set(_mosquitto, topic.c_str(), static_cast<int>(strlen(LOST)), LOST, 0, false);
    }

    bool Mqtt::verifyConnection() const {
        if (_isConnected) return true;
        printf("Connection lost. Reconnecting\n");
        mosquitto_reconnect(_mosquitto);
        return waitForConnection();
    }

    bool Mqtt::waitForConnection() const { 
        // we don't wait more than 5 seconds
        constexpr int MAX_WAIT_DECISECONDS = 50;
        for (int i = 0; i < MAX_WAIT_DECISECONDS; i++) {
            if (_isConnected) return true;
            if (!*_keepGoing) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    return _isConnected;
    }
}