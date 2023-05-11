#ifndef DHT_H
#define DHT_H

#include "SensorData.h"
#include <cstdint>

class Dht {
public:
    Dht(uint8_t pin, SensorData* sensorData);
    bool read();
    void begin();
    float readHumidity();
    float readTemperature();

private:
    uint8_t _pin = 4;
    SensorData* _sensorData;
    uint32_t _lastReadTime = 0;
    bool _conversionOk = false;
    float _humidity = 0.0f;
    float _temperature = 0.0f;
};



#endif