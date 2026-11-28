#include "CanDevice.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct CanDeviceTest : public VcanTest, public CanDevice {
    CanDeviceTest() : CanDevice(1, test_can_master()) {};
protected:
    void update_current(int16_t current) override {};
};

TEST_F(CanDeviceTest, IsAliveInitialisesTrue) {
    EXPECT_TRUE(is_alive());
}

TEST_F(CanDeviceTest, TargetInitialisesZero) {
    EXPECT_EQ(0, get_can_target());
}

TEST_F(CanDeviceTest, CurrentInitialisesZero) {
    EXPECT_EQ(0, get_can_current());
}

TEST_F(CanDeviceTest, SetTargetWorks) {
    EXPECT_EQ(0, get_can_target());
    set_target(100);
    EXPECT_EQ(100, get_can_target());
}

TEST_F(CanDeviceTest, TargetChangedGoesTrue) {
    EXPECT_FALSE(get_can_target_changed());
    set_target(100);
    EXPECT_TRUE(get_can_target_changed());
}

TEST_F(CanDeviceTest, TargetChangedResetsToFalse) {
    EXPECT_FALSE(get_can_target_changed());
    set_target(100);
    EXPECT_TRUE(get_can_target_changed());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(get_can_target_changed());
}
