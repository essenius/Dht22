#ifndef MQTT1_H
#define MQTT1_H

#include <mosquittopp.h>
#include <unordered_map>
#include <string>
#include <unistd.h>
#include "Config.h"
#include "ISender.h"

class Mqtt : public mosqpp::mosquittopp {
public:
    Mqtt(const Config* config);
    ~Mqtt();
    bool begin();
    bool isConnected() { return _isConnected; }
    bool connect1();
    void shutdown();
    bool verifyConnection();
    bool waitForConnection();

private:
    const Config* _config;
    const char* _broker = nullptr;
    const char* _caCert = nullptr;
    const char* _user = nullptr;
    const char* _password = nullptr;
    int _port = 1883;
    int _keepAliveSeconds = 60;
    bool _isConnected = false;
    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_publish(int mid) override;
    void on_log(int level, const char* str) override;

//    bool sendString(const char *item, const char *message);

};

#endif