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
#include <iostream>

/// @brief Processes an edge signal from the DHT22 sensor. Called from the callback function, so it needs to be fast.
/// The DHT22 sends 40 bits of data, which include:
/// - 16 bits for humidity, stored as 10 times the value (i.e. 652 means 65.2 %).
/// - 16 bits for temperature, stored as 10 times the value, and the most significant bit is the sign (so hex 8065 = negative 101 dec = -10.1 °C).
/// - 8 bits for a checksum to verify data integrity.
///
/// Each bit is transmitted as:
/// - A low signal for 50 microseconds (the reference duration), followed by:
/// - A high signal for 26–28 microseconds (i.e. clearly less than the reference duration) to represent a "0".
/// - A high signal for 70 microseconds (i.e. clearly more than the reference duration) to represent a "1".
/// The data starts after 4 edges, so we have a total of 84 edges to process.
/// 
/// If all edges have been processed, it validates the checksum and sets the sensor state to Done or Error.
// Check out https://cdn-shop.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf for the details.
///
/// @param levelIn The new signal level (0 for low, 1 for high, 2 for timeout).
/// @param timestamp The timestamp of the detected edge.
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

    //
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
