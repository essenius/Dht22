#ifndef MQTT1_H
#define MQTT1_H

#include <mosquitto.h>
#include "Config.h"

namespace Queuing {
    void onConnect(mosquitto* mosq, void* obj, int rc);
    void onDisconnect(mosquitto* mosq, void* obj, int returnCode);
    void onPublish(mosquitto* mosq, void* obj, int messageId);
    void onLog(mosquitto* mosq, void* obj, int level, const char* str);

    class Mqtt {
    public:
        explicit Mqtt(const Config* config, bool* keepGoing);
        ~Mqtt();
        bool begin();
        int errorCode() const { return _errorCode; }
        bool isConnected() const { return _isConnected; }
        bool publish(const std::string& topic, const std::string& message, bool retain = false);
        void setWill(const std::string& topic) const;
        bool verifyConnection();
        bool waitForConnection() const;
        friend void onConnect(mosquitto* mosq, void* obj, int rc);
        friend void onDisconnect(mosquitto* mosq, void* obj, int returnCode);
        friend void onPublish(mosquitto* mosq, void* obj, int messageId);
        friend void onLog(mosquitto* mosq, void* obj, int level, const char* str);


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
        bool* _keepGoing = nullptr;
        bool firstConnect();
        void setConnected(bool connected) { _isConnected = connected; }
        void setErrorCode(int returnCode) { _errorCode = returnCode; }
    };
}

#endif