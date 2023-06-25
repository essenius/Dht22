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
    void shutdown() const;
    bool waitForNextMeasurement(volatile bool& keepGoing);
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
    void log(const std::string& message, bool trace = false) const;
    void reportResult(bool success);
};

#endif