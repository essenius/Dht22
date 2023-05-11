#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cstdint>
#include <stdio.h>
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
    void addEdge(int level, uint32_t timestamp);
    bool isDone() const;
    bool isReading() const;
    float getHumidity();
    SensorState getState() const;
    float getTemperature();
    uint16_t getWordAtIndex(const uint8_t index);
    void initRead(uint32_t timestamp);
    void print();

private:
    int _currentIndex = 0;
    std::array<uint8_t, BYTES> _data;
    int _overrunCount = 0;
    uint32_t _previousTime;
    uint32_t _referenceDuration;
    SensorState _state = SensorState::Timeout; // any state not Done or Reading
};

#endif