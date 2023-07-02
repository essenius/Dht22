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

#include "SensorData.h"
#include <cmath>
#include <cstdio>

/// @brief Add an edge to the data. Called from callback, so needs to be fast.
/// Compare the low and high cycle times to see if the bit is a 0 or 1. 
/// Reference (low) duration is 50-55 us, High for 1 is 70-75us, high for 0 is 25-30us. 
/// At the end, sets state to SensorState::Done if the checksum is correct, otherwise SensorState::Error
/// @param levelIn the new level (0 or 1 for bit values, 2 for timeout)
/// @param timestamp the timestamp where the edge was detected
void SensorData::addEdge(const int levelIn, const uint32_t timestamp) {

    if (!isReading()) {
        _overrunCount++;
        return;
    }
    const uint32_t duration = timestamp - _previousTime;
    switch (levelIn) {
        // move from 1 to 0, so we just had a data bit
        case 0:
            {
                // we should only get this after the reference bit
                if (_referenceDuration == 0) {
                    _anomaly++;
                    return;
                }
	        const auto dataIndex = (_currentIndex - START_EDGE) / 16;
                // shift left by 1
                _data[dataIndex] *= 2;
                if (duration > _referenceDuration) {
                    _data[dataIndex] += 1;
                }
                _referenceDuration = 0;
            }
            break;
        // move from 0 to 1, so we just had a reference bit
        case 1:
            _referenceDuration = duration;
            break;
        // any other value indicates a timeout
        default:
            printf("Timeout\n");
            _state = SensorState::Timeout;
            return;
    }

    _previousTime = timestamp;
    _currentIndex++;

    if(_currentIndex >= EDGES) {
        const auto checksum = (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF;
        _state = checksum == _data[4] ? SensorState::Done : SensorState::ReadError;
    }
}

float SensorData::getHumidity() const {
    if (!isDone()) {
        return NAN;
    }
    return static_cast<float>(getWordAtIndex(0)) * 0.1f;
}

SensorState SensorData::getState() const {
    return _state;
}

float SensorData::getTemperature() const {
    if (!isDone()) {
        return NAN;
    }
    const auto word = getWordAtIndex(2);
    const bool isNegative = word & 0x8000;
    return static_cast<float>(word & 0x7FFF) * 0.1f * (isNegative ? -1 : 1);
}

uint16_t SensorData::getWordAtIndex(const uint8_t index) const {
    const auto result = _data[index] * 256u + _data[index + 1];
    return static_cast<uint16_t>(result);
}

void SensorData::initRead(const uint32_t timestamp) {
    _previousTime = timestamp;
    _referenceDuration = 0;
    _currentIndex = 0;
    _anomaly = 0;
    _state = SensorState::Reading;
    for(int i = 0; i < BYTES; i++) {
        _data[i] = 0;
    }
}

bool SensorData::isDone() const {
    return _state == SensorState::Done;
}

bool SensorData::isReading() const {
    return _state == SensorState::Reading;
}
