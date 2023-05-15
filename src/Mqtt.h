#ifndef MQTT1_H
#define MQTT1_H

#include <mosquittopp.h>
#include <unordered_map>
#include <string>
#include <unistd.h>
#include "Config.h"

class Mqtt : public mosqpp::mosquittopp {
public:
    Mqtt(const Config* config);
    ~Mqtt();
    int begin();
    bool connect1();
    bool sendFloat(const char* item, float value);
    bool isConnected() { return _isConnected; }

private:
    const Config* _config;
    const char* _broker = nullptr;
    const char* _caCert = nullptr;
    const char* _user = nullptr;
    const char* _password = nullptr;
    int _port = 1883;
    const char* _topicTemplate = nullptr;
    int _keepAliveSeconds = 60;
    bool _isConnected = false;

    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_publish(int mid) override;
};

#endif