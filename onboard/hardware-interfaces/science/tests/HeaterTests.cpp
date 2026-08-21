#include "Heater.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* heater_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* heater_test_can_master = new RoverCanMaster(*heater_test_can_bus, 1);

struct HeaterTest : ::testing::Test, public Heater {
    HeaterTest() : Heater(1, *heater_test_can_master) {};
};

TEST_F(HeaterTest, CurrentTemperatureInitialisesZero) {
    EXPECT_EQ(0, getCurrentTemperature());
}

TEST_F(HeaterTest, TargetTemperatureInitialisesZero) {
    EXPECT_EQ(0, getTargetTemperature());
}

TEST_F(HeaterTest, SetTargetTemperatureWorks) {
    setTargetTemperature(100);
    EXPECT_EQ(100, getTargetTemperature());
    setTargetTemperature(50);
    EXPECT_EQ(50, getTargetTemperature());
}

TEST_F(HeaterTest, SetTargetTemperatureRejectsNegative) {
    setTargetTemperature(-100);
    EXPECT_GE(0, getTargetTemperature());
}

TEST_F(HeaterTest, UpdateCurrentWorks) {
    EXPECT_EQ(0, getCurrentTemperature());
    updateCurrent(100);
    EXPECT_EQ(100, getCurrentTemperature());
    updateCurrent(50);
    EXPECT_EQ(50, getCurrentTemperature());
}

TEST_F(HeaterTest, UpdateCurrentAcceptsNegative) {
    EXPECT_EQ(0, getCurrentTemperature());
    updateCurrent(-100);
    EXPECT_EQ(-100, getCurrentTemperature());
}