#include "GenericMotor.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* generic_motor_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* generic_motor_test_can_master = new RoverCanMaster(*generic_motor_test_can_bus, 1);

struct GenericMotorTest : public ::testing::Test, public GenericMotor {
    GenericMotorTest() : GenericMotor(1, *generic_motor_test_can_master) {}
};

TEST_F(GenericMotorTest, TargetRpmInitialisesZero) {
    EXPECT_EQ(0, getTargetRpm());
}

TEST_F(GenericMotorTest, CurrentRpmInitialisesZero) {
    EXPECT_EQ(0, getCurrentRpm());
}

TEST_F(GenericMotorTest, SetRpmWorks) {
    setRpm(100);
    EXPECT_EQ(100, getTargetRpm());
    setRpm(50);
    EXPECT_EQ(50, getTargetRpm());
}

TEST_F(GenericMotorTest, SetRpmAcceptsNegative) {
    setRpm(-100);
    EXPECT_EQ(-100, getTargetRpm());
}

TEST_F(GenericMotorTest, StopWorks) {
    setRpm(100);
    EXPECT_NE(0, getTargetRpm());
    stop();
    EXPECT_EQ(0, getTargetRpm());
}

TEST_F(GenericMotorTest, UpdateCurrentWorks) {
    EXPECT_EQ(0, getCurrentRpm());
    updateCurrent(100);
    EXPECT_EQ(100, getCurrentRpm());
}