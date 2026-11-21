#include "GenericMotor.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct GenericMotorTest : public VcanTest, public GenericMotor {
    GenericMotorTest() : GenericMotor(1, test_can_master()) {}
};

TEST_F(GenericMotorTest, TargetRpmInitialisesZero) {
    EXPECT_EQ(0, get_target_rpm());
}

TEST_F(GenericMotorTest, CurrentRpmInitialisesZero) {
    EXPECT_EQ(0, get_current_rpm());
}

TEST_F(GenericMotorTest, SetRpmWorks) {
    set_rpm(100);
    EXPECT_EQ(100, get_target_rpm());
    set_rpm(50);
    EXPECT_EQ(50, get_target_rpm());
}

TEST_F(GenericMotorTest, SetRpmAcceptsNegative) {
    set_rpm(-100);
    EXPECT_EQ(-100, get_target_rpm());
}

TEST_F(GenericMotorTest, StopWorks) {
    set_rpm(100);
    EXPECT_NE(0, get_target_rpm());
    stop();
    EXPECT_EQ(0, get_target_rpm());
}

TEST_F(GenericMotorTest, UpdateCurrentWorks) {
    EXPECT_EQ(0, get_current_rpm());
    update_current(100);
    EXPECT_EQ(100, get_current_rpm());
}
