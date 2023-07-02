// Copyright 2023 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#include <pigpio.h>
#include <cstdio> 
#include <cmath>
#include "Dht.h"

// must run as sudo

// we can't read the sensor more often than every 2 seconds
constexpr uint32_t MIN_INTERVAL_MICROS =  2 * 1000 * 1000;
// Measurement transmission should take no more than 7.5 ms. Give 2.5 ms extra.  
constexpr int READ_TIMEOUT_MILLIS = 10;
// we're waiting at least 79 * 40 us, i.e. about 3000 us. Usually it's 5-6 ms
constexpr uint32_t MINIMUM_READ_TIME_MICROS = 3000;
// don't check too often, but also not too seldom
constexpr uint32_t WAIT_INTERVAL_MICROS = 500;
// when powering down, wait at least 50 ms before powering up
constexpr uint32_t SHUTDOWN_TIME_MICROS = 50000;
constexpr int MAX_CONSECUTIVE_FAILURES = 10;

Dht::Dht(SensorData* sensorData, Config* config) :  _sensorData(sensorData), _config(config) {}

Dht::~Dht() {
    log("Dht destructor", true);
    shutdown();
}

bool Dht::begin() {
    _config->setIfExists("dataPin", &_dataPin);
    _config->setIfExists("powerPin", &_powerPin);
    auto cfg = gpioCfgGetInternals();
    cfg |= PI_CFG_NOSIGHANDLER;  
    gpioCfgSetInternals(cfg);
    if (gpioInitialise() < 0) {
        log("could not initialize GPIO. Terminating and re-initializing...", false);
        gpioTerminate();
        if (gpioInitialise() < 0) return false;
    }

    gpioSetMode(_powerPin, PI_OUTPUT);
    gpioWrite(_powerPin, PI_HIGH);
    _startupTime = gpioTick();
    printf("%u: Initialized GPIO v%u, HW revision: %u\n", _startupTime, gpioVersion(), gpioHardwareRevision());
    _lastReadTime = _startupTime - MIN_INTERVAL_MICROS;
    _nextScheduledRead = _startupTime + MIN_INTERVAL_MICROS;
    _consecutiveFailures = 0;
    return true;
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
    log("Failed to get sensor value", false);
    if (_consecutiveFailures > MAX_CONSECUTIVE_FAILURES) {
        log("Too many consecutive failures. Resetting sensor.", false);
        reset();
    }
}

void Dht::shutdown() const {
    log("Shutting down DHT", false);
    gpioWrite(_powerPin, PI_LOW);
    gpioTerminate();
}

void Dht::reset() {
    shutdown();
    gpioDelay(SHUTDOWN_TIME_MICROS);
    begin();
    // Make sure that read() uses the previous reading (which is mist likely NAN).
    // the sensor was just powered up, so we can't read it right away.
    _lastReadTime = _startupTime;
}

bool Dht::waitForNextMeasurement(volatile bool& keepGoing) {
	const uint32_t currentTime = gpioTick();
    if (currentTime == static_cast<uint32_t>(PI_NOT_INITIALISED)) return false;
    log("Waiting", true);
    int32_t waitTime;
    while (waitTime = static_cast<int32_t>(_nextScheduledRead - gpioTick()), waitTime > 0 && keepGoing) {
        const auto timeToSleep = std::min(waitTime, 100000);
        gpioDelay(timeToSleep);
    }
    // printf("Waited %u us for next measurement\n", gpioTick() - currentTime);
    if (waitTime < -10000) {
        // if we're off more than 10 milliseconds, recalibrate. This could happen after connection issues
        printf("Recalibrating. Next scheduled read was %u us ago. New is %u plus time for this print command\n", -waitTime, gpioTick());
        _nextScheduledRead = gpioTick();
    }
    return true;
}

void pinCallback([[maybe_unused]] int gpio, int level, uint32_t tick, void *userData) {
	auto*data = static_cast<SensorData*>(userData);
    data->addEdge(level, tick);
}

/// @brief Read the sensor and store the result in the class variables. Expects the sensor to be powered up (does not wait).
/// @return whether a valid result is available. A cached result of less than two seconds old is considered valid.
bool Dht::read() {
    // printf("Reading\n");
    const uint32_t currentTime = gpioTick();
    if ((static_cast<int32_t>(currentTime - _lastReadTime) < static_cast<int32_t>(MIN_INTERVAL_MICROS)) && (static_cast<int32_t>(currentTime - _nextScheduledRead) < 0)) {
        // printf("Using cache: current=%u last=%u, next=%u, result=%d\n", currentTime, _lastReadTime, _nextScheduledRead, _conversionOk);
        return _conversionOk; 
    }

    _lastReadTime = currentTime;
    _nextScheduledRead += MIN_INTERVAL_MICROS;
    // printf("Reading.. (last=%u, next=%u, diff=%d)\n", _lastReadTime, _nextScheduledRead, static_cast<int32_t>(_nextScheduledRead - _lastReadTime));

    // Send start signal.  See DHT data sheet for full signal diagram:
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
    gpioDelay(MINIMUM_READ_TIME_MICROS);
    while (_sensorData->isReading()) {
        log("Waiting for data", true);
        gpioDelay(WAIT_INTERVAL_MICROS);
    } 
    waitTime = gpioTick() - waitTime;

    // stop the watch dog and the callback
    log("Stopping watchdog", true);
    gpioSetWatchdog(_dataPin, 0);
    log("Stopping callback", true);
    gpioSetAlertFuncEx(_dataPin, nullptr, nullptr);
    // printf("Waited %u ns for data\n", waitTime);

    log("getting humidity", true);
    _humidity = _sensorData->getHumidity();
    log("getting temperature", true);
    _temperature = _sensorData->getTemperature();
    const auto anomalies = _sensorData->getAnomalyCount();
    if (anomalies > 0) {
        printf("Found %d anomalies\n", anomalies);
    }
    _conversionOk = _sensorData->isDone();
    reportResult(_conversionOk);
    return _conversionOk;
}

void Dht::log(const std::string& message, bool trace) const {
    if (!trace || (trace && _trace)) {
        printf("%u: %s\n", gpioTick(), message.c_str());
    }
}
