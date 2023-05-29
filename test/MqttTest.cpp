#include <gtest/gtest.h>

#include "Mqtt.h"

class MqttTest : public ::testing::Test {};

TEST_F(MqttTest, NoTLSNoAuth) {
    Config config;
    const char* configData = "device=pi230265\nbroker=mqtt.org\nport=1883\n";
    config.begin(configData);
    Mqtt mqtt(&config);
    EXPECT_TRUE(mqtt.begin()) << "Connect OK";
    bool keepGoing = true;
    EXPECT_TRUE(mqtt.waitForConnection(keepGoing)) << "Wait for connection OK";
    EXPECT_TRUE(mqtt.isConnected()) << "Is connected";
    EXPECT_TRUE(mqtt.verifyConnection()) << "Verify connection OK";
    mqtt.disconnect();
    EXPECT_FALSE(mqtt.isConnected()) << "Is not connected";
    EXPECT_TRUE(mqtt.verifyConnection());
    EXPECT_TRUE(mqtt.isConnected()) << "Is connected after verify";
    mqtt.shutdown();
    EXPECT_FALSE(mqtt.isConnected()) << "Is not connected after shutdown";
}