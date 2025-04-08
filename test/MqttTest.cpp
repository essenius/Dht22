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

#include <thread>
#include <chrono>

#include "Mqtt.h"

class MqttTest : public ::testing::Test {};

volatile bool keepGoing = true;

TEST_F(MqttTest, NonexistingServer) {
    Config config;

    const auto configData = "device=pi230265\nbroker=nonexisting.org\nport=-1883\n";
    config.begin(configData);
    queuing::Mqtt mqtt(&config, &keepGoing);
    EXPECT_FALSE(mqtt.begin()) << "Connect not OK (negative port)";
}

TEST_F(MqttTest, WrongCert) {
    Config config;
    const auto configData = "device=pi230265\nbroker=nonexisting.org\nport=1883\ncaCert=nonexisting\n";
    config.begin(configData);
    queuing::Mqtt mqtt(&config, &keepGoing);
    EXPECT_FALSE(mqtt.begin()) << "Connect not OK (invalid cert)";
}

TEST_F(MqttTest, WithAuth) {
    Config config;
    const auto configData = "device=pi230265\nbroker=nonexisting.org\nport=1883\nuser=ro\npassword=readonly\n";
    config.begin(configData);
    queuing::Mqtt mqtt(&config, &keepGoing);
    EXPECT_FALSE(mqtt.begin()) << "Connect not OK";

    EXPECT_TRUE(mqtt.errorCode() != 0) << "Non-zero error code";
    EXPECT_FALSE(mqtt.isConnected()) << "Not connected: rc=" << mqtt.errorCode();
    EXPECT_FALSE(mqtt.verifyConnection()) << "Verify connection not OK";
    keepGoing = false;
    EXPECT_FALSE(mqtt.waitForConnection()) << "Wait for connection not OK";
    // force connect since we can't do it for real
    onConnect(nullptr, &mqtt, 1);
    EXPECT_EQ(1, mqtt.errorCode()) << "Error code 1 taken over";
    EXPECT_FALSE(mqtt.isConnected()) << "Not connected";
    queuing::onConnect(nullptr, &mqtt, 0);
    EXPECT_EQ(0, mqtt.errorCode()) << "Error code 0";
    EXPECT_TRUE(mqtt.isConnected()) << "Connected";
    EXPECT_TRUE(mqtt.verifyConnection()) << "Connection verified";
    keepGoing = true;
    EXPECT_TRUE(mqtt.waitForConnection()) << "Wait for connection OK";
}