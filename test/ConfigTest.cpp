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

