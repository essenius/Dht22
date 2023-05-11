#include "SensorData.h"
#include <cmath>

void SensorData::setIdle() {
    _state = SensorState::Idle;
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

/// @brief Add an edge to the data. Called from callback, so needs to be fast
/// @param levelIn the new level (0 or 1)
/// @param timestamp the timestamp that the edge was detected
void SensorData::addEdge(int levelIn, uint32_t timestamp) {

    // this should not happen, so making visible if it does (i.e. bug). Remove later.
    if (readEnded()) {
        printf("Already ended! level=%d, timestamp=%u\n", levelIn, timestamp);
        return;
    }

    _duration[_currentIndex] = timestamp - _previousTime;
    _level[_currentIndex] = levelIn;
    _currentIndex++;
    _previousTime = timestamp;
    if (levelIn == PI_TIMEOUT) {
        _state = SensorState::Timeout;
        return;
    }
    if(_currentIndex >= EDGES) {
        _state = SensorState::Transmitted;
    }
}

bool SensorData::readEnded() {
    return _state != SensorState::Idle && _state != SensorState::Reading;
}

bool SensorData::isDone() {
    return _state == SensorState::Done;
}

void SensorData::print() {
    printf("SensorData: state=%d, currentIndex=%d, previousTime=%u\n", _state, _currentIndex, _previousTime);
    float totalLow = 0; 
    float totalHigh0 = 0;
    float totalHigh1 = 0;
    int highCount = 0;
    int totalCount = 0;
    for (int i = START_EDGE; i < _currentIndex; i+=2) {
        totalCount++;
        totalLow += _duration[i];
        if (_duration[i] > _duration[i + 1]) {
            totalHigh0 += _duration[i + 1];
        } else {
            highCount++;
            totalHigh1 += _duration[i + 1];
        }
    }
    printf("Totals: count=%2d\tHighs=%2d\nAverages: low %.1f\thigh-0 %.1f\thigh-1 %.1f\n", totalCount, highCount, float(totalLow) / totalCount, float(totalHigh0) / (totalCount - highCount), float(totalHigh1) / highCount);

}

// Only call this if the state is SensorState::Transmitted, i.e. just after completing the transmission
float SensorData::getHumidity() {
    convertToBytes();
    if (!isDone()) {
        return NAN;
    }
    float f = ((uint16_t)_data[0]) << 8 | _data[1];
    return f * 0.1;
}

// Only call this if the state is SensorState::Transmitted, i.e. just after completing the transmission
float SensorData::getTemperature() {
    convertToBytes();
    if (!isDone()) {
        return NAN;
    }
    float f = ((uint16_t)(_data[2] & 0x7F)) << 8 | _data[3];
    bool isNegative = _data[2] & 0x80;
    return f * 0.1 * (isNegative ? -1 : 1);
}

/// @brief Decode the data into bytes. Only works if the status is SensorState::Transmitted
/// @details This is called after the transmission has completed, and the data is in the duration and level arrays.
/// Compare the low and high cycle times to see if the bit is a 0 or 1. 
/// Low duration is 50-55 us, High for 1 is 70-75us, high for 0 is 25-30us. 
/// Sets state to SensorState::Done if the checksum is correct, otherwise SensorState::Error
void SensorData::convertToBytes() {
    if (_state != SensorState::Transmitted) {
        return;
    }

    for (int i = START_EDGE; i < EDGES; i += 2) {
        const auto index = (i - START_EDGE) / 16;
        _data[index] <<= 1;
        if (_duration[i + 1] > _duration[i]) {
            _data[index] |= 1;
        }
    }
    
    auto checksum = (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF;
    _state = checksum == _data[4] ? SensorState::Done : SensorState::Error;
}
