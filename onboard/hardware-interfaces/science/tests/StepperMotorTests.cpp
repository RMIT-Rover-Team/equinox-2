#include "StepperMotor.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* stepper_motor_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* stepper_motor_test_can_master = new RoverCanMaster(*stepper_motor_test_can_bus, 1);

struct StepperMotorTest : public ::testing::Test, public StepperMotor {
    StepperMotorTest() : StepperMotor(1, *stepper_motor_test_can_master) {}
    void testUpdateCurrent(const int16_t new_value) {
        updateCurrent(new_value);
    }
};

TEST_F(StepperMotorTest, CurrentStepsInitialisesZero) {
    EXPECT_EQ(0, getCurrentSteps());
}

TEST_F(StepperMotorTest, TargetStepsInitialisesZero) {
    EXPECT_EQ(0, getTargetSteps());
}

TEST_F(StepperMotorTest, SetTargetStepsAcceptsPositive) {
    setTargetSteps(100);
    EXPECT_EQ(100, getTargetSteps());
}

TEST_F(StepperMotorTest, SetTargetStepsAcceptsNegative) {
    setTargetSteps(-100);
    EXPECT_EQ(-100, getTargetSteps());
}

TEST_F(StepperMotorTest, updateCurrentAcceptsPositive) {
    testUpdateCurrent(100);
    EXPECT_EQ(100, getCurrentSteps());
}

TEST_F(StepperMotorTest, updateCurrentAcceptsNegative) {
    testUpdateCurrent(-100);
    EXPECT_EQ(-100, getCurrentSteps());
}

TEST_F(StepperMotorTest, StopSetsTargetEqualToCurrent) {
    testUpdateCurrent(100);
    EXPECT_EQ(0, getTargetSteps());
    stop();
    EXPECT_EQ(100, getTargetSteps());

    testUpdateCurrent(-100);
    EXPECT_EQ(100, getTargetSteps());
    stop();
    EXPECT_EQ(-100, getTargetSteps());
}