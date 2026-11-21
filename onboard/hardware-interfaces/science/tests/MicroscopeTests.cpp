#include "Microscope.h"
#include <gtest/gtest.h>

#include "SciencePayload.h"
#include "VcanTest.h"


struct MicroscopeTest : public VcanTest, public Microscope {
    MicroscopeTest() : Microscope(1, test_can_master()) {};
};

TEST_F(MicroscopeTest, CurrentHeightInitialisesZero) {
    EXPECT_EQ(0, get_current_height());
}

TEST_F(MicroscopeTest, TargetHeightInitialisesZero) {
    EXPECT_EQ(0, get_target_height());
}

TEST_F(MicroscopeTest, CurrentSwivelInitialisesZero) {
    EXPECT_EQ(0, get_current_lateral());
}

TEST_F(MicroscopeTest, TargetSwivelInitialisesZero) {
    EXPECT_EQ(0, get_target_lateral());
}

TEST_F(MicroscopeTest, SetHeightAcceptsPositive) {
    set_height(100.0);
}

TEST_F(MicroscopeTest, SetHeightAcceptsNegative) {
    set_height(-100.0);
}

TEST_F(MicroscopeTest, SetSwivelAcceptsPositive) {
    set_lateral(100.0);
}

TEST_F(MicroscopeTest, SetSwivelAcceptsNegative) {
    set_lateral(-100.0);
}
