#include <cstring>
#include <stdexcept>
#include <cmath>

#include "Mqtt.h"
#include <mosquittopp.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

Mqtt::Mqtt(const Config* config) : mosquittopp(config->getEntry("device").c_str()), _config(config) {}

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
        if (tls_set(_caCert) != MOSQ_ERR_SUCCESS) {
            std::cerr << "failed\n";
            return false;
        }
    }
    if (strlen(_user) > 0) {
        printf("setting user %s\n", _user);
        if (username_pw_set(_user, _password) != MOSQ_ERR_SUCCESS) {
            std::cerr << "failed\n";
            return false;
        }
    }

    printf("Connecting to %s:%d, with keepalive %d\n", _broker, _port, _keepAliveSeconds);
    if (int rc = connect(_broker, _port, _keepAliveSeconds); rc != MOSQ_ERR_SUCCESS) {
        _errorCode = rc;
        std::cerr << "Connect failed, error: " << rc << "/" << mosqpp::strerror(rc) << "\n";
        return false;
    }

    printf("starting loop\n");
    loop_start();
    
    std::cout << "Started loop\n";

    return true;
}

void Mqtt::shutdown() {
    std::cout << "##-Mqtt Shutdown-##" << std::endl; 
    disconnect();
    loop_stop();
}

Mqtt::~Mqtt() {
    std::cout << "##-Mqtt Destructor-##" << std::endl; 
    shutdown();
}

void Mqtt::on_connect(int rc) {
    _errorCode = rc;
    _isConnected = (rc == MOSQ_ERR_SUCCESS);
    if (_isConnected) {
        std::cout << "## Connected" << std::endl;
    }
    else {
        std::cout << "## Failed connecting - code " << rc << ": " << mosqpp::strerror(rc) << std::endl;
    }
}

void Mqtt::on_disconnect(int rc) {
    std::cout << " ##-Disconnected from Broker-## " << rc << ": " << mosqpp::strerror(rc) << std::endl;
    _errorCode = rc;
    _isConnected = false;
}

void Mqtt::on_publish(int mid) {
    std::cout << "## - Message published successfully: " << mid << std::endl;
}

void Mqtt::on_log(int level, const char* str) {
    std::cout << "## - Log: " << level << "-" << str << std::endl;
}

bool Mqtt::verifyConnection() {
    if (_isConnected) return true;
    printf("Connection lost. Reconnecting\n");
    reconnect();
    bool keepGoing = true;
    return waitForConnection(keepGoing);
}

bool Mqtt::waitForConnection(bool& keepGoing) const { 
    // we don't wait more than 5 seconds
    constexpr int MAX_WAIT_DECISECONDS = 50;
    for (int i = 0; i < MAX_WAIT_DECISECONDS; i++) {
        if (_isConnected) return true;
        if (!keepGoing) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
   return _isConnected;
}