#ifndef DHT_H
#define DHT_H

#include "SensorData.h"
#include <cstdint>

class Dht {
public:
    Dht(SensorData* sensorData, uint8_t powerPin = 17, uint8_t dataPin = 4);
    ~Dht();
    void begin();
    float readHumidity();
    float readTemperature();
    void reset();

private:
    uint8_t _powerPin = 17;
    uint8_t _dataPin = 4;
    SensorData* _sensorData;
    uint32_t _startupTime = 0;
    uint32_t _lastReadTime = 0;
    bool _conversionOk = false;
    float _humidity = 0.0f;
    float _temperature = 0.0f;
    unsigned int _consecutiveFailures = 0;

    bool read();
    void reportResult(bool success);
};



#endif