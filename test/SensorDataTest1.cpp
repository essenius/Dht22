#include <gtest/gtest.h>

#include "SensorData.h"
class SensorDataTest : public ::testing::Test {};


TEST_F(SensorDataTest, init) {
    SensorData sensorData;
    EXPECT_EQ(sensorData.state, SensorState::Idle);
    sensorData.init(55);
    EXPECT_EQ(sensorData.state, SensorState::Activated);
    EXPECT_EQ(sensorData.currentIndex, 0);
    EXPECT_EQ(sensorData.previousTime, 55);
}

TEST_F(SensorDataTest, addEdgeHappyPathAllZero) {
    SensorData sensorData;
    sensorData.init(0);
    int level = 0;
    for (int i = 0; i < EDGES; i++) {
        sensorData.addEdge(level, 100 * (i+1));
        level = 1 - level;
    }
    sensorData.print();

    EXPECT_EQ(sensorData.currentIndex, EDGES) << "currentIndex";
    EXPECT_EQ(sensorData.state, SensorState::Done) << "state";
    for (int i = 0; i < EDGES; i++) {
        EXPECT_EQ(sensorData.duration[i], 100) << "duration for index " << i;
        EXPECT_EQ(sensorData.level[i], i % 2) << "Level for index " << i;
    }    
}

TEST_F(SensorDataTest, convertToBytes1) {
    SensorData sensorData;
    uint32_t timestamp = 0;
    sensorData.init(timestamp);
    timestamp += 40;
    sensorData.addEdge(0, timestamp);
    timestamp += 80;
    sensorData.addEdge(1, timestamp);
    timestamp += 80;
    sensorData.addEdge(0, timestamp);
    for (int i = 0; i < EDGES / 2 - 1; i++) {
        timestamp += 50;
        sensorData.addEdge(1, timestamp);
        timestamp += (i % 2 == 0 || i == EDGES / 2 - 2) ? 30: 70;
        sensorData.addEdge(0, timestamp);
    }

    EXPECT_TRUE(sensorData.convertToBytes());
    EXPECT_EQ(sensorData.data[0], 0x55) << "First";
    EXPECT_EQ(sensorData.data[1], 0x55) << "Second";
    EXPECT_EQ(sensorData.data[2], 0x55) << "Third";
    EXPECT_EQ(sensorData.data[3], 0x55) << "Fourth";
    EXPECT_EQ(sensorData.data[4], 0x54) << "Fifth";
}

TEST_F(SensorDataTest, addEdgeTimeout) {
    SensorData sensorData;
    uint32_t timestamp = 0;
    sensorData.init(timestamp);
    int level = 0;
    for (int i = 0; i < EDGES / 2; i++) {
        timestamp += 100;
        sensorData.addEdge(level, timestamp);
        level = 1 - level;
    }
    timestamp += 10000;
    sensorData.addEdge(PI_TIMEOUT, timestamp);

    EXPECT_EQ(sensorData.currentIndex, EDGES / 2 + 1) << "currentIndex";
    EXPECT_EQ(sensorData.state, SensorState::Timeout) << "timeout";
    EXPECT_FALSE(sensorData.convertToBytes());
}