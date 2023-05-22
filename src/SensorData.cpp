#include "SensorData.h"
#include <cmath>

/// @brief Add an edge to the data. Called from callback, so needs to be fast.
/// Compare the low and high cycle times to see if the bit is a 0 or 1. 
/// Reference (low) duration is 50-55 us, High for 1 is 70-75us, high for 0 is 25-30us. 
/// At the end, sets state to SensorState::Done if the checksum is correct, otherwise SensorState::Error
/// @param levelIn the new level (0 or 1 for bit values, 2 for timeout)
/// @param timestamp the timestamp where the edge was detected
void SensorData::addEdge(int levelIn, uint32_t timestamp) {

    if (!isReading()) {
        _overrunCount++;
        return;
    }

    int32_t duration = timestamp - _previousTime;

    switch (levelIn) {
        // move from 1 to 0, so we just had a data bit
        case 0:
            { 
                auto dataIndex = (_currentIndex - START_EDGE) / 16;
                _data[dataIndex] <<= 1;
                if (duration > _referenceDuration) {
                    _data[dataIndex] |= 1;
                }
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
        auto checksum = (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF;
        _state = checksum == _data[4] ? SensorState::Done : SensorState::ReadError;
    }
}

float SensorData::getHumidity() {
    if (!isDone()) {
        return NAN;
    }
    return static_cast<float>(getWordAtIndex(0)) * 0.1f;
}

SensorState SensorData::getState() const {
    return _state;
}

float SensorData::getTemperature() {
    if (!isDone()) {
        return NAN;
    }
    auto word = getWordAtIndex(2);
    bool isNegative = word & 0x8000;
    return static_cast<float>(word & 0x7FFF) * 0.1f * (isNegative ? -1 : 1);
}

uint16_t SensorData::getWordAtIndex(const uint8_t index) {
    auto result = _data[index] * 256u + _data[index + 1];
    return static_cast<uint16_t>(result);
}

void SensorData::initRead(uint32_t timestamp)
{
    _previousTime = timestamp;
    _currentIndex = 0;
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
