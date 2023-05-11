#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cstdint>
#include <stdio.h>
#include <array>
#include <pigpio.h>

constexpr int EDGES = 84;
constexpr int BYTES = 5;
constexpr int START_EDGE = 4;

enum class SensorState {
    Idle,
    Reading,
    Timeout,
    Transmitted,
    Error,
    Done
};

class SensorData {
public:
    void setIdle();
    void initRead(uint32_t timestamp);
    void addEdge(int level, uint32_t timestamp);
    bool readEnded();
    bool isDone();
    void print();
    float getTemperature();
    float getHumidity();

private:
    uint32_t _previousTime;
    std::array<uint32_t, EDGES> _duration;
    std::array<int, EDGES> _level;
    int _currentIndex = 0;
    SensorState _state = SensorState::Idle;
    std::array<uint8_t, BYTES> _data;

    void convertToBytes();

};

#endif