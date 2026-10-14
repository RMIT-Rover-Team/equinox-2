#include "Drill.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* drill_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* drill_test_can_master = new RoverCanMaster(*drill_test_can_bus, 1);

struct DrillTest : ::testing::Test, public Drill {
    DrillTest() : Drill(1, *drill_test_can_master) {};
};

TEST_F(DrillTest, CurrentRpmInitialisesZero) {
    EXPECT_EQ(0, getCurrentRpm());
}

TEST_F(DrillTest, CurrentHeightInitialisesZero) {
    EXPECT_EQ(0, getCurrentHeight());
}

TEST_F(DrillTest, TargetHeightInitialisesZero) {
    EXPECT_EQ(0, getTargetHeight());
};

TEST_F(DrillTest, CanSetTargetHeight) {
    setHeight(100);
    EXPECT_EQ(100, getTargetHeight());
    setHeight(-100);
    EXPECT_EQ(-100, getTargetHeight());
}

TEST_F(DrillTest, TargetRpmInitialisesZero) {
    EXPECT_EQ(0, getTargetRpm());
}

TEST_F(DrillTest, CanSetTargetRpm) {
    start(100);
    EXPECT_EQ(100, getTargetRpm());
    start();
    EXPECT_EQ(DRILL_RUNNING_RPM, getTargetRpm());
}

TEST_F(DrillTest, CanStopDrill) {
    start(100);
    EXPECT_NE(0, getTargetRpm());
    stop();
    EXPECT_EQ(0, getTargetRpm());
}