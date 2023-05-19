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
// when powering down, wait at least 50 ms before powering up
constexpr uint32_t SHUTDOWN_TIME_MICROS = 50000;
constexpr int MAX_CONSECUTIVE_FAILURES = 5;

Dht::Dht(SensorData* sensorData, Config* config) :  _sensorData(sensorData), _config(config) {}

void Dht::begin() {
    _config->setIfExists("dataPin", &_dataPin);
    _config->setIfExists("powerPin", &_powerPin);
    gpioSetMode(_powerPin, PI_OUTPUT);
    gpioWrite(_powerPin, PI_HIGH);
    _startupTime = gpioTick();
    _nextScheduledRead = _startupTime + MIN_INTERVAL_MICROS;
    _consecutiveFailures = 0;
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

void Dht::reportResult(const bool success) {
    if (success) {
        _consecutiveFailures = 0;
        return;
    }
    _consecutiveFailures++;
    printf("Failed to get sensor value\n");
    if (_consecutiveFailures > MAX_CONSECUTIVE_FAILURES) {
        printf("Too many consecutive failures. Resetting sensor.\n");
        reset();
    }
}

void Dht::shutdown() {
    gpioWrite(_powerPin, PI_LOW);
    gpioTerminate();
}

void Dht::reset() {
    gpioWrite(_powerPin, PI_LOW);
    gpioDelay(SHUTDOWN_TIME_MICROS);
    begin();
}

void Dht::waitForNextMeasurement() {
    uint32_t currentTime = gpioTick();
    if (currentTime < _nextScheduledRead) {
        gpioDelay(_nextScheduledRead - currentTime);
    } else if (currentTime - _nextScheduledRead > 10000) {
        // if we're off more than 10 milliseconds, recalibrate. This could happen after connection issues
        _nextScheduledRead = currentTime;
    }
}

void pinCallback(int gpio, int level, uint32_t tick, void *userdata) {
    SensorData *data = (SensorData *)userdata;
    data->addEdge(level, tick);
}

/// @brief Read the sensor and store the result in the class variables. Expects the sensor to be powered up (does not wait).
/// @return whether a valid result is available. A cached result of less than two seconds old is considered valid.
bool Dht::read() {

    uint32_t currentTime = gpioTick();
    if ((currentTime < _nextScheduledRead) && (currentTime - _lastReadTime < MIN_INTERVAL_MICROS)) {
        printf("Using cache: current=%u last=%u, next=%u, result=%d\n", currentTime, _lastReadTime, _nextScheduledRead, _conversionOk);
        return _conversionOk; 
    }

    printf("Reading..\n");
    _lastReadTime = currentTime;
    _nextScheduledRead += MIN_INTERVAL_MICROS;

    // Send start signal.  See DHT datasheet for full signal diagram:
    //   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

    // Pull up the data line and wait a millisecond.
    gpioSetMode(_dataPin, PI_INPUT);
    gpioSetPullUpDown(_dataPin, PI_PUD_UP);
    gpioDelay(1000);

    // Set data line low for 1.1 ms (which satisfies "at least 1ms" for DHT22)

    gpioSetMode(_dataPin, PI_OUTPUT);
    gpioWrite(_dataPin, PI_LOW);

    gpioDelay(1100); 

    _sensorData->initRead(gpioTick());

    // Pull up the data line again, and let the sensor take over.
    gpioSetMode(_dataPin, PI_INPUT);
    gpioSetPullUpDown(_dataPin, PI_PUD_UP);

    // monitor the pin for changes 
    gpioSetAlertFuncEx(_dataPin, pinCallback, _sensorData);
    // time out if we don't get a change on time
    gpioSetWatchdog(_dataPin, READ_TIMEOUT_MILLIS); 

    uint32_t waitTime = gpioTick();
    // wait for the callback to complete reading.
    gpioDelay(MINUMIM_READ_TIME_MICROS);
    while (_sensorData->isReading()) {
        gpioDelay(WAIT_INTERVAL_MICROS);
    } 
    waitTime = gpioTick() - waitTime;

    // stop the watch dog and the callback
    gpioSetWatchdog(_dataPin, 0);
    gpioSetAlertFuncEx(_dataPin, nullptr, nullptr);
    printf("Waited %u ns\n", waitTime);

    _humidity = _sensorData->getHumidity();
    _temperature = _sensorData->getTemperature();
    _conversionOk = _sensorData->isDone();
    reportResult(_conversionOk);
    return _conversionOk;
}
