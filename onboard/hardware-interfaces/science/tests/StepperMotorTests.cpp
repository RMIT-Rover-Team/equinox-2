#include "StepperMotor.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct StepperMotorTest : public VcanTest, public StepperMotor {
    StepperMotorTest() : StepperMotor(1, test_can_master()) {}
    void test_update_current(const int16_t new_value) {
        update_current(new_value);
    }
};

TEST_F(StepperMotorTest, CurrentStepsInitialisesZero) {
    EXPECT_EQ(0, get_current_steps());
}

TEST_F(StepperMotorTest, TargetStepsInitialisesZero) {
    EXPECT_EQ(0, get_target_steps());
}

TEST_F(StepperMotorTest, SetTargetStepsAcceptsPositive) {
    set_target_steps(100);
    EXPECT_EQ(100, get_target_steps());
}

TEST_F(StepperMotorTest, SetTargetStepsAcceptsNegative) {
    set_target_steps(-100);
    EXPECT_EQ(-100, get_target_steps());
}

TEST_F(StepperMotorTest, updateCurrentAcceptsPositive) {
    test_update_current(100);
    EXPECT_EQ(100, get_current_steps());
}

TEST_F(StepperMotorTest, updateCurrentAcceptsNegative) {
    test_update_current(-100);
    EXPECT_EQ(-100, get_current_steps());
}

TEST_F(StepperMotorTest, StopSetsTargetEqualToCurrent) {
    test_update_current(100);
    EXPECT_EQ(0, get_target_steps());
    stop();
    EXPECT_EQ(100, get_target_steps());

    test_update_current(-100);
    EXPECT_EQ(100, get_target_steps());
    stop();
    EXPECT_EQ(-100, get_target_steps());
}
