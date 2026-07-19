#include "Microscope.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* microscope_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* microscope_test_can_master = new RoverCanMaster(*microscope_test_can_bus, 1);

struct MicroscopeTest : public testing::Test, public Microscope {
    MicroscopeTest() : Microscope(1, *microscope_test_can_master) {};
};

TEST_F(MicroscopeTest, CurrentHeightInitialisesZero) {
    EXPECT_EQ(0, getCurrentHeight());
}

TEST_F(MicroscopeTest, TargetHeightInitialisesZero) {
    EXPECT_EQ(0, getTargetHeight());
}

TEST_F(MicroscopeTest, CurrentSwivelInitialisesZero) {
    EXPECT_EQ(0, getCurrentSwivel());
}

TEST_F(MicroscopeTest, TargetSwivelInitialisesZero) {
    EXPECT_EQ(0, getTargetSwivel());
}

TEST_F(MicroscopeTest, SetHeightAcceptsPositive) {
    setHeight(100.0);
}

TEST_F(MicroscopeTest, SetHeightAcceptsNegative) {
    setHeight(-100.0);
}

TEST_F(MicroscopeTest, SetSwivelAcceptsPositive) {
    setSwivel(100.0);
}

TEST_F(MicroscopeTest, SetSwivelAcceptsNegative) {
    setSwivel(-100.0);
}