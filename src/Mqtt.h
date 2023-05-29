#ifndef MQTT1_H
#define MQTT1_H

#include <mosquittopp.h>
#include <unistd.h>
#include "Config.h"

class Mqtt : public mosqpp::mosquittopp {
public:
    explicit Mqtt(const Config* config);
    ~Mqtt();
    bool begin();
    bool isConnected() const { return _isConnected; }
    void shutdown();
    bool verifyConnection();
    bool waitForConnection(bool& keepGoing) const;

private:
    const Config* _config;
    const char* _broker = nullptr;
    const char* _caCert = nullptr;
    const char* _user = nullptr;
    const char* _password = nullptr;
    int _port = 1883;
    int _keepAliveSeconds = 60;
    bool _isConnected = false;
    bool firstConnect();
    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_publish(int mid) override;
    void on_log(int level, const char* str) override;
};

#endif