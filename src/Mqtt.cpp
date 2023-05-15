#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cmath>

#include "Mqtt.h"
#include <mosquittopp.h>

#include <fstream>
#include <iostream>


using namespace std;

Mqtt::Mqtt(const Config* config) : mosquittopp(config->getEntry("id")) {
    _config = config;
};

int Mqtt::begin() {
    printf("Begin, id=%s\n", _config->getEntry("id"));
    _caCert = _config->getEntry("caCert");
    _broker = _config->getEntry("broker", "localhost");
    _port = std::stoi(_config->getEntry("port", "1883"));
    _user = _config->getEntry("user");
    _password = _config->getEntry("password");
    _topicTemplate = _config->getEntry("topicTemplate");
    _keepAliveSeconds = std::stoi(_config->getEntry("keepAliveSeconds", "60"));
    if (_topicTemplate == nullptr) {
        std::cerr << "Failed to find topicTemplate in config file\n";
        return false;
    }
    return connect1();
}

bool Mqtt::connect1() {
    if (_isConnected) return true;
    if (_caCert != nullptr) {
        printf("setting ca cert %s\n", _caCert);
        if (tls_set(_caCert) != MOSQ_ERR_SUCCESS) {
            std::cerr << "failed\n";
            return false;
        };
    }
    if (_user != nullptr) {
        printf("setting user %s\n", _user);
        if (username_pw_set(_user, _password) != MOSQ_ERR_SUCCESS) {
            std::cerr << "failed\n";
            return false;
        }
    }
    printf("Connecting to %s:%d, with keepalive %d\n", _broker, _port, _keepAliveSeconds);
    if (connect(_broker, _port, _keepAliveSeconds) == MOSQ_ERR_ERRNO) {
        std::cerr << "Failed\n";
        return false;
    }

    loop_start();
    return true;
}

Mqtt::~Mqtt() {
  loop_stop();
  cout << "##-Destructor-##" << endl; 
}

void Mqtt::on_connect(int rc) {
    _isConnected = (rc == 0);
    if (rc == 0) {
        cout << " ##-Connected with Broker-## " << rc << std::endl;
    }
    else {
        cout << "##-Unable to Connect Broker-## " << rc << std::endl;
    }
}

bool Mqtt::sendFloat(const char* item, float value) {
    constexpr const char* ERROR_ITEM = "error";
    char topic[256];
    char message[256];
    if(isnan(value)) {
        snprintf(topic, sizeof(topic), _topicTemplate, ERROR_ITEM);
        strcpy(message, item);
    } else {
        snprintf(topic, sizeof(topic), _topicTemplate, item);
        snprintf(message, sizeof(message), "%.1f", value);
    }
    printf("Sending %s to %s\n", message, topic);
    int returnValue = publish(nullptr, topic, strlen(message), message, 0, false);
    return (returnValue == MOSQ_ERR_SUCCESS);
}

void Mqtt::on_disconnect(int rc) {
    cout << " ##-Disconnected from Broker-## " << rc << std::endl;
    _isConnected = false;
}

void Mqtt::on_publish(int mid) {
    cout << "## - Message published successfully: " << mid << endl;
}


