#include "Drill.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct DrillTest : public VcanTest, public Drill {
    DrillTest() : Drill(1, test_can_master()) {};
};

TEST_F(DrillTest, CurrentRpmInitialisesZero) {
    EXPECT_EQ(0, get_current_rpm());
}

TEST_F(DrillTest, CurrentHeightInitialisesZero) {
    EXPECT_EQ(0, get_current_height());
}

TEST_F(DrillTest, TargetHeightInitialisesZero) {
    EXPECT_EQ(0, get_target_height());
};

TEST_F(DrillTest, CanSetTargetHeight) {
    set_height(100);
    EXPECT_EQ(100, get_target_height());
    set_height(-100);
    EXPECT_EQ(-100, get_target_height());
}

TEST_F(DrillTest, TargetRpmInitialisesZero) {
    EXPECT_EQ(0, get_target_rpm());
}

TEST_F(DrillTest, CanSetTargetRpm) {
    start(100);
    EXPECT_EQ(100, get_target_rpm());
    start();
    EXPECT_EQ(DRILL_RUNNING_RPM, get_target_rpm());
}

TEST_F(DrillTest, CanStopDrill) {
    start(100);
    EXPECT_NE(0, get_target_rpm());
    stop();
    EXPECT_EQ(0, get_target_rpm());
}
