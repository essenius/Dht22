#include <gtest/gtest.h>

#include <thread>
#include <chrono>

#include "Mqtt.h"

class MqttTest : public ::testing::Test {};

TEST_F(MqttTest, NonexistingServer) {
    Config config;
    const char* configData = "device=pi230265\nbroker=nonexisting.org\nport=-1883\n";
    config.begin(configData);
    Mqtt mqtt(&config);
    EXPECT_FALSE(mqtt.begin()) << "Connect not OK (negative port)";
}

TEST_F(MqttTest, WrongCert) {
    Config config;
    const char* configData = "device=pi230265\nbroker=nonexisting.org\nport=1883\ncaCert=nonexisting\n";
    config.begin(configData);
    Mqtt mqtt(&config);
    EXPECT_FALSE(mqtt.begin()) << "Connect not OK (invalid cert)";
}

TEST_F(MqttTest, WithAuth) {
    Config config;
    const char* configData = "device=pi230265\nbroker=nonexisting.org\nport=1883\nuser=pi\npassword=secret\n";
    config.begin(configData);
    Mqtt mqtt(&config);
    EXPECT_TRUE(mqtt.begin()) << "Connect OK";
    EXPECT_EQ(0, mqtt.errorCode()) << "Error code still 0";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(mqtt.errorCode() != 0) << "Non-zero error code";
    EXPECT_FALSE(mqtt.isConnected()) << "Not connected: rc=" << mqtt.errorCode();
    EXPECT_FALSE(mqtt.verifyConnection()) << "Verify connection not OK";
    bool keepGoing = false;
    EXPECT_FALSE(mqtt.waitForConnection(keepGoing)) << "Wait for connection not OK";
    // force connect since we can't do it for real
    mqtt.on_connect(1);
    EXPECT_EQ(1, mqtt.errorCode()) << "Error code 1 taken over";
    EXPECT_FALSE(mqtt.isConnected()) << "Not connected";
    mqtt.on_connect(0);
    EXPECT_EQ(0, mqtt.errorCode()) << "Error code 0";
    EXPECT_TRUE(mqtt.isConnected()) << "Connected";
    EXPECT_TRUE(mqtt.verifyConnection()) << "Connection verified";
    keepGoing = true;
    EXPECT_TRUE(mqtt.waitForConnection(keepGoing)) << "Wait for connection OK";
}