#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cmath>

#include "Mqtt.h"
#include <mosquittopp.h>

#include <fstream>
#include <iostream>


using namespace std;

constexpr const char* STATE = "state";

Mqtt::Mqtt(const Config* config) : mosquittopp(config->getEntry("device")) {
    _config = config;
};

bool Mqtt::begin() {
    _caCert = _config->getEntry("caCert");
    _broker = _config->getEntry("broker", "localhost");
    _config->setIfExists("port", &_port);
    _user = _config->getEntry("user");
    _password = _config->getEntry("password");
    _config->setIfExists("keepAliveSeconds", &_keepAliveSeconds);

    return connect1();
}


bool Mqtt::connect1() {
    if (_isConnected) return true;
//    char willTopic[256];
//    snprintf(willTopic, sizeof(willTopic), _topicTemplate, STATE);
//    constexpr const char* LOST = "lost";
//    will_set(willTopic, strlen(LOST), LOST);
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
    if (auto rc = connect(_broker, _port, _keepAliveSeconds) == MOSQ_ERR_ERRNO) {
        std::cerr << "Failed, error: " << strerror(rc) << "\n";
        return false;
    }

    printf("starting loop\n");
    loop_start();
    
    std::cout << "Started loop\n";

    return true;
}

void Mqtt::shutdown() {
    disconnect();
    loop_stop();
}

Mqtt::~Mqtt() {
  cout << "##-Destructor-##" << endl; 
}

/*
bool Mqtt::sendMessage(std::string topic, const char* message) {
    int returnValue = publish(nullptr, topic.c_str(), strlen(message), message, 0, false);
    return (returnValue == MOSQ_ERR_SUCCESS);
}


bool Mqtt::sendString(const char* item, const char* message) {
    char topic[256];
    snprintf(topic, sizeof(topic), _topicTemplate, item);
    printf("Sending %s to %s\n", message, topic);
    return sendMessage(topic, message);
}

bool Mqtt::sendFloat(const char* item, float value) {
    constexpr const char* ERROR_ITEM = "error";
    if(isnan(value)) {
        return sendString(ERROR_ITEM, item);
    } 
    char message[20];
    snprintf(message, sizeof(message), "%.1f", value);
    return sendString(item, message);
}
*/

void Mqtt::on_connect(int rc) {
    _isConnected = (rc == MOSQ_ERR_SUCCESS);
    if (_isConnected) {
        cout << "## Connected" << std::endl;
    }
    else {
        cout << "## Failed connecting - code " << rc << std::endl;
    }
}

void Mqtt::on_disconnect(int rc) {
    cout << " ##-Disconnected from Broker-## " << rc << std::endl;
    _isConnected = false;
}

void Mqtt::on_publish(int mid) {
    cout << "## - Message published successfully: " << mid << endl;
}

void Mqtt::on_log(int level, const char* str) {
    cout << "## - Log: " << level << "-" << str << endl;
}

bool Mqtt::verifyConnection() {
    if (_isConnected) return true;
    printf("Connection lost. Reconnecting\n");
    reconnect();
    return waitForConnection();
}

bool Mqtt::waitForConnection() {
    // we don't wait more than 5 seconds
    constexpr int MAX_WAIT_DECISECONDS = 50;
    int repeatCount = 0;
    while(!_isConnected && repeatCount++ < MAX_WAIT_DECISECONDS) { 
      usleep(100000); 
   }
   return repeatCount < MAX_WAIT_DECISECONDS;
}