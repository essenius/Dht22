#ifndef DHT_H
#define DHT_H

#include "SensorData.h"
#include "Config.h"
#include <cstdint>

class Dht {
public:
    Dht(SensorData* sensorData, Config* config);
    ~Dht();
    bool begin();
    float readHumidity();
    float readTemperature();
    void reset();
    void shutdown();
    bool waitForNextMeasurement(bool& keepGoing);
    void trace() { _trace = true; }

private:
    uint8_t _powerPin = 4;
    uint8_t _dataPin = 17;
    SensorData* _sensorData;
    Config* _config;
    uint32_t _startupTime = 0;
    uint32_t _lastReadTime = 0;
    uint32_t _nextScheduledRead = 0;
    bool _conversionOk = false;
    float _humidity = 0.0f;
    float _temperature = 0.0f;
    bool _trace = false;
    unsigned int _consecutiveFailures = 0;

    bool read();
    void log(const std::string& message, bool trace = false);
    void reportResult(bool success);
};

#endif