#ifndef SENSORDATA_H
#define SENSORDATA_H

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

private:
    int _currentIndex = 0;
    std::array<uint8_t, BYTES> _data = {};
    int _overrunCount = 0;
    uint32_t _previousTime = 0;
    uint32_t _referenceDuration = 0;
    SensorState _state = SensorState::Timeout; // any state not Done or Reading
};

#endif