#include <gtest/gtest.h>

#include "Dht.h"
#include "SensorData.h"

class DhtTest : public ::testing::Test {};

TEST_F(DhtTest, first) {
    SensorData sensorData;
    Dht dht(4, &sensorData);
}

