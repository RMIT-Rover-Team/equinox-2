#include "Heater.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct HeaterTest : public VcanTest, public Heater {
    HeaterTest() : Heater(1, test_can_master()) {};
};

TEST_F(HeaterTest, CurrentTemperatureInitialisesZero) {
    EXPECT_EQ(0, get_current_temperature());
}

TEST_F(HeaterTest, TargetTemperatureInitialisesZero) {
    EXPECT_EQ(0, get_target_temperature());
}

TEST_F(HeaterTest, SetTargetTemperatureWorks) {
    set_target_temperature(100);
    EXPECT_EQ(100, get_target_temperature());
    set_target_temperature(50);
    EXPECT_EQ(50, get_target_temperature());
}

TEST_F(HeaterTest, SetTargetTemperatureRejectsNegative) {
    set_target_temperature(-100);
    EXPECT_GE(0, get_target_temperature());
}

TEST_F(HeaterTest, UpdateCurrentWorks) {
    EXPECT_EQ(0, get_current_temperature());
    update_current(100);
    EXPECT_EQ(100, get_current_temperature());
    update_current(50);
    EXPECT_EQ(50, get_current_temperature());
}

TEST_F(HeaterTest, UpdateCurrentAcceptsNegative) {
    EXPECT_EQ(0, get_current_temperature());
    update_current(-100);
    EXPECT_EQ(-100, get_current_temperature());
}
