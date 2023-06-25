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

#include "Config.h"

class ConfigTest : public ::testing::Test {};

TEST_F(ConfigTest, noDevice) {
    Config config;
    const char* configData = "deviceName=pi\nbroker=mqtt\nport=1883\n";
    config.begin(configData);
    EXPECT_STREQ("pi", config.getEntry("deviceName")) << "deviceName OK";
    EXPECT_STREQ("mqtt", config.getEntry("broker")) << "broker OK";
    EXPECT_STREQ("1883", config.getEntry("port")) << "port OK";
    EXPECT_EQ(nullptr, config.getEntry("device")) <<  "device not found";
}

TEST_F(ConfigTest, deviceInConfig) {
    Config config;
    const char* configData = "device=pi\nbroker=mqtt\nport=1883\n";
    config.begin(configData);
    EXPECT_STREQ("pi", config.getEntry("device")) << "device OK";
    EXPECT_STREQ("mqtt", config.getEntry("broker")) << "broker OK";
    EXPECT_STREQ("1883", config.getEntry("port")) << "port OK";

    std::string buffer;
    config.setIfExists("device", &buffer);
    EXPECT_EQ("pi", buffer);
    int port;
    config.setIfExists("port", &port);
    EXPECT_EQ(1883, port);
}

TEST_F(ConfigTest, deviceFromOS) {
    Config config;
    const char* configData = "broker=mqtt\nport=1883\n";
    config.begin(configData, "mypi");
    EXPECT_STREQ("mqtt", config.getEntry("broker")) << "broker OK";
    EXPECT_STREQ("1883", config.getEntry("port")) << "port OK";
    EXPECT_STREQ("mypi", config.getEntry("device")) <<  "device taken from OS";
}

TEST_F(ConfigTest, deviceFromOSoverruled) {
    Config config;
    const char* configData = "device=pi\nbroker=mqtt\nport=1883\n";
    config.begin(configData, "should_not_see");
    EXPECT_STREQ("mqtt", config.getEntry("broker")) << "broker OK";
    EXPECT_STREQ("1883", config.getEntry("port")) << "port OK";
    EXPECT_STREQ("pi", config.getEntry("device")) <<  "device taken from config";
}

