#include <pigpio.h>
#include <stdio.h> 
#include <cstring>
#include <cmath>
#include "Dht.h"

// must run as sudo

// we can't read the sensor more often than every 2 seconds
constexpr uint32_t MIN_INTERVAL_MICROS =  2 * 1000 * 1000;
// Measurement transmission should take no more than 7.5 ms. Give 2.5 ms extra.  
constexpr int READ_TIMEOUT_MILLIS = 10;
// we're waiting at least 79 * 40 us, i.e. about 3000 us. Usually it's 5-6 ms
constexpr uint32_t MINUMIM_READ_TIME_MICROS = 3000;
// don't check too often, but also not too seldom
constexpr uint32_t WAIT_INTERVAL_MICROS = 500;

Dht::Dht(uint8_t pin, SensorData* sensorData) : _pin(pin), _sensorData(sensorData) {}

void Dht::begin() {
    _lastReadTime = gpioTick() - MIN_INTERVAL_MICROS;
    _sensorData->setIdle();
}

float Dht::readHumidity() {
    if (read()) {
        return _humidity;
    }
    return NAN;
}

float Dht::readTemperature() {
    if (read()) {
        return _temperature;
    }
    return NAN;
}

void pinCallback(int gpio, int level, uint32_t tick, void *userdata) {
    SensorData *data = (SensorData *)userdata;
    data->addEdge(level, tick);
}

bool Dht::read() {

    // Check if sensor was read less than two seconds ago and return early
    // to use last reading.
    uint32_t currenttime = gpioTick();

    if (currenttime - _lastReadTime < MIN_INTERVAL_MICROS) {
      printf("Too early: c=%u l=%u, diff=%u\n", currenttime, _lastReadTime, currenttime - _lastReadTime);
      return _conversionOk; 
    }
    printf("Reading..\n");
    _lastReadTime = currenttime;

    // Send start signal.  See DHT datasheet for full signal diagram:
    //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

    // Pull up the data line and wait a millisecond.
    gpioSetMode(_pin, PI_INPUT);
    gpioSetPullUpDown(_pin, PI_PUD_UP);
    gpioDelay(1000);

    // Set data line low for 1.1 ms (which satisfies "at least 1ms" for DHT22)

    gpioSetMode(_pin, PI_OUTPUT);
    gpioWrite(_pin, PI_LOW);

    gpioDelay(1100); 

    _sensorData->initRead(gpioTick());

    // Pull up the data line again, and let the sensor take over.
    gpioSetMode(_pin, PI_INPUT);
    gpioSetPullUpDown(_pin, PI_PUD_UP);

    // monitor the pin for changes 
    gpioSetAlertFuncEx(_pin, pinCallback, _sensorData);
    // time out if we don't get a change on time
    gpioSetWatchdog(_pin, READ_TIMEOUT_MILLIS); 

    uint32_t waitTime = gpioTick();
    // wait for the callback to complete reading.
    gpioDelay(MINUMIM_READ_TIME_MICROS);
    while (!_sensorData->readEnded()) {
        gpioDelay(WAIT_INTERVAL_MICROS);
    } 
    waitTime = gpioTick() - waitTime;

    // stop the watch dog and the callback
    gpioSetWatchdog(_pin, 0);
    gpioSetAlertFuncEx(_pin, nullptr, nullptr);
    printf("Waited %u ns\n", waitTime);
    _sensorData->print();

    _humidity = _sensorData->getHumidity();
    _temperature = _sensorData->getTemperature();
    _conversionOk = _sensorData->isDone();
    if (_conversionOk) {
      printf("Done: humidity=%.1f\ttemperature=%.1f\n", _humidity, _temperature);

    } else {
      printf("Issue\n");
    }
    return _conversionOk;
}
