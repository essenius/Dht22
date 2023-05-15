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
// when powering down, wait at least 50 ms before powering up again
constexpr uint32_t SHUTDOWN_TIME_MICROS = 50000;
constexpr int MAX_CONSECUTIVE_FAILURES = 5;

Dht::Dht(SensorData* sensorData, uint8_t powerPin, uint8_t dataPin) :  _sensorData(sensorData), _powerPin(powerPin), _dataPin(dataPin) {}

Dht::~Dht() {
    gpioWrite(_powerPin, PI_LOW);
}

void Dht::begin() {
    gpioSetMode(_powerPin, PI_OUTPUT);
    gpioWrite(_powerPin, PI_HIGH);
    _startupTime = gpioTick();
    _lastReadTime = gpioTick();
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

void Dht::reset() {
    gpioWrite(_powerPin, PI_LOW);
    gpioDelay(SHUTDOWN_TIME_MICROS);
    begin();
}

void pinCallback(int gpio, int level, uint32_t tick, void *userdata) {
    SensorData *data = (SensorData *)userdata;
    data->addEdge(level, tick);
}

bool Dht::read() {

    uint32_t currenttime = gpioTick();
    // if we just started, wait for the sensor to start up
    auto timeSinceStartup = currenttime - _startupTime;
    if (timeSinceStartup < MIN_INTERVAL_MICROS) {
        printf("Waiting for startup: c=%u s=%u, diff=%u\n", currenttime, _startupTime, timeSinceStartup);
        gpioDelay(MIN_INTERVAL_MICROS - timeSinceStartup);
    } else {
        // if we've run longer, check if sensor was read less than two seconds ago. If so, return the last reading.
        auto timeSinceLastRead = currenttime - _lastReadTime;
        if (timeSinceLastRead < MIN_INTERVAL_MICROS) {
            printf("Using cache: c=%u l=%u, diff=%u, result=%d\n", currenttime, _lastReadTime, timeSinceLastRead, _conversionOk);
            return _conversionOk; 
        }
    }
    printf("Reading..\n");
    _lastReadTime = currenttime;

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
    _sensorData->print();

    _humidity = _sensorData->getHumidity();
    _temperature = _sensorData->getTemperature();
    _conversionOk = _sensorData->isDone();
    reportResult(_conversionOk);
    return _conversionOk;
}
