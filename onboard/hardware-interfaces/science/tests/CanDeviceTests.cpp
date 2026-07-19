#include "CanDevice.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "SocketCanWrapper.h"

auto* can_device_test_can_bus = new WrappedCANBus(CAN_BUS_NAME);
auto* can_device_test_can_master = new RoverCanMaster(*can_device_test_can_bus, 1);

struct CanDeviceTest : public ::testing::Test, public CanDevice {
    CanDeviceTest() : CanDevice(1, *can_device_test_can_master) {};
protected:
    void updateCurrent(int16_t current) override {};
};

TEST_F(CanDeviceTest, IsAliveInitialisesTrue) {
    EXPECT_TRUE(isAlive());
}

TEST_F(CanDeviceTest, TargetInitialisesZero) {
    EXPECT_EQ(0, getCanTarget());
}

TEST_F(CanDeviceTest, CurrentInitialisesZero) {
    EXPECT_EQ(0, getCanCurrent());
}

TEST_F(CanDeviceTest, SetTargetWorks) {
    EXPECT_EQ(0, getCanTarget());
    setTarget(100);
    EXPECT_EQ(100, getCanTarget());
}

TEST_F(CanDeviceTest, TargetChangedGoesTrue) {
    EXPECT_FALSE(getCanTargetChanged());
    setTarget(100);
    EXPECT_TRUE(getCanTargetChanged());
}

TEST_F(CanDeviceTest, TargetChangedResetsToFalse) {
    EXPECT_FALSE(getCanTargetChanged());
    setTarget(100);
    EXPECT_TRUE(getCanTargetChanged());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(getCanTargetChanged());
}