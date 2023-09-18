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

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <cstdint>
#include <array>

constexpr int EDGES = 84;
constexpr int BYTES = 5;
constexpr int START_EDGE = 4;

enum class SensorState {
    Reading,
    Timeout,
    ReadError,
    Done
};

class SensorData {
public:
    void addEdge(int levelIn, uint32_t timestamp);
    [[nodiscard]] bool isDone() const;
    [[nodiscard]] bool isReading() const;
    [[nodiscard]] float getHumidity() const;
    [[nodiscard]] SensorState getState() const;
    [[nodiscard]] float getTemperature() const;
    [[nodiscard]] uint16_t getWordAtIndex(const uint8_t index) const;
    void initRead(uint32_t timestamp);
    int getAnomalyCount() { return _anomaly; }
private:
    int _currentIndex = 0;
    std::array<uint8_t, BYTES> _data = {};
    int _overrunCount = 0;
    unsigned int _anomaly = 0;
    uint32_t _previousTime = 0;
    uint32_t _referenceDuration = 0;
    uint16_t _lastGoodHumidity = 0;
    SensorState _state = SensorState::Timeout; // any state not Done or Reading
};

#endif
