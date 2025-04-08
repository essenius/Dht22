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

#include <gtest/gtest.h>
#include <cmath>
#include "SensorData.h"

class SensorDataTest : public ::testing::Test {
    public:
    void simulateDataStream(SensorData& sensorData, SensorState expectedState, bool isPositive) const {
        EXPECT_EQ(SensorState::Timeout, sensorData.getState()) << "Initial state is Timeout";
        uint32_t timestamp = 0;
        sensorData.initRead(timestamp);
        EXPECT_TRUE(sensorData.isReading()) << "State is Reading";
        timestamp += 5;
        sensorData.addEdge(1, timestamp);
        timestamp += 20;
        sensorData.addEdge(0, timestamp);
        timestamp += 80;
        sensorData.addEdge(1, timestamp);
        timestamp += 80;
        sensorData.addEdge(0, timestamp);
        for (int i = 0; i < 40; i++) {
            timestamp += 50;
            sensorData.addEdge(1, timestamp);
            EXPECT_TRUE(sensorData.isReading()) << "State is Reading at point " << i;

            if (expectedState == SensorState::Timeout && i == 20) {
                sensorData.addEdge(2, timestamp);
                break;
            }

            bool isOff;

            if (expectedState == SensorState::Done) {
                if (isPositive) {
                    isOff = (i % 2 == 0 || i == 39);
                } else {
                    isOff = (i % 2 == 1 || i == 38);
                }
            } else {
                isOff = false;
            }
            timestamp += isOff ? 30: 70;
            sensorData.addEdge(0, timestamp);
        }
    }
};


TEST_F(SensorDataTest, addEdgeHappyPathAllZero) {
    SensorData sensorData;
    sensorData.initRead(0);
    int level = 0;
    for (int i = 0; i <= EDGES; i++) {
        sensorData.addEdge(level, 100 * (i + 1));
        level = 1 - level;
    }

    EXPECT_TRUE(sensorData.isDone()) << "Done";
    EXPECT_FLOAT_EQ(0.0f, sensorData.getTemperature()) << "Temperature";
    EXPECT_FLOAT_EQ(0.0f, sensorData.getHumidity()) << "Humidity";
}

TEST_F(SensorDataTest, CorrectDataPositiveTemperature) {
    SensorData sensorData;
    simulateDataStream(sensorData, SensorState::Done, true);
    EXPECT_TRUE(sensorData.isDone()) << "is Done";
    EXPECT_EQ(0x5555, sensorData.getWordAtIndex(0)) << "Word @ 0";
    EXPECT_EQ(0x5555, sensorData.getWordAtIndex(2)) << "Word @ 2";

    // Data is 0x5555 = 21845. Value is divided by 10, so 2184.5
    EXPECT_FLOAT_EQ(2184.5, sensorData.getTemperature()) << "Temperature OK";
    EXPECT_FLOAT_EQ(2184.5, sensorData.getHumidity()) << "Humidity OK";
}

TEST_F(SensorDataTest, CorrectDataNegativeTemperature) {
    SensorData sensorData;
    simulateDataStream(sensorData, SensorState::Done, false);
    EXPECT_TRUE(sensorData.isDone()) << "is Done";
    EXPECT_EQ(0xAAAA, sensorData.getWordAtIndex(0)) << "Word @ 0";
    EXPECT_EQ(0xAAAA, sensorData.getWordAtIndex(2)) << "Word @ 2";
    // Data is 0xAAAA = 43690. For Temperature, the most significant bit is the sign.
    // So there the value is -0x2AAA = -10922. Then again divided by 10.
    EXPECT_FLOAT_EQ(-1092.2f, sensorData.getTemperature()) << "Temperature OK";
    EXPECT_FLOAT_EQ(4369.0f, sensorData.getHumidity()) << "Humidity OK";
}

TEST_F(SensorDataTest, IncorrectData) {
    SensorData sensorData;
    simulateDataStream(sensorData, SensorState::ReadError, true);
    EXPECT_EQ(0xFFFF, sensorData.getWordAtIndex(0)) << "Word @ 0";
    EXPECT_EQ(0xFFFF, sensorData.getWordAtIndex(2)) << "Word @ 2";

    EXPECT_EQ(SensorState::ReadError, sensorData.getState()) << "State is ReadError";
    EXPECT_TRUE(std::isnan(sensorData.getTemperature())) << "Temperature is NaN";
    EXPECT_TRUE(std::isnan(sensorData.getHumidity())) << "Temperature is NaN";
    EXPECT_EQ(SensorState::ReadError, sensorData.getState()) << "State did not change from Error";
}

TEST_F(SensorDataTest, Timeout) {
    SensorData sensorData;
    simulateDataStream(sensorData, SensorState::Timeout, true);
    EXPECT_EQ(SensorState::Timeout, sensorData.getState()) << "State is Timeout";
    EXPECT_TRUE(std::isnan(sensorData.getTemperature())) << "Temperature is NaN";
    EXPECT_TRUE(std::isnan(sensorData.getHumidity())) << "Temperature is NaN";
    EXPECT_EQ(SensorState::Timeout, sensorData.getState()) << "State did not change from Timeout";
}
