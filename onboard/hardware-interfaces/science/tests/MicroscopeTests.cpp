#include <gtest/gtest.h>

#include <Microscope.h>
#include "VcanTest.h"

struct MicroscopeTest : public VcanTest {
    static constexpr CanAddress height_addr{GroupID::Payload, 0x01};
    static constexpr CanAddress lateral_addr{GroupID::Payload, 0x02};
    Microscope microscope{height_addr, lateral_addr, test_can_master()};
};

TEST_F(MicroscopeTest, SetHeightSendsStepsToHeightMotor) {
    microscope.set_height(100);
    const CANFrame frame = expect_frame(height_addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt16 frame sent to the height motor";
    EXPECT_EQ(static_cast<int16_t>(100 * MICROSCOPE_HEIGHT_MOTOR_RATIO), int16_at(frame, 0));
}

TEST_F(MicroscopeTest, SetHeightAcceptsNegative) {
    microscope.set_height(-100);
    const CANFrame frame = expect_frame(height_addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc);
    EXPECT_EQ(static_cast<int16_t>(-100 * MICROSCOPE_HEIGHT_MOTOR_RATIO), int16_at(frame, 0));
}

TEST_F(MicroscopeTest, SetLateralSendsStepsToLateralMotor) {
    microscope.set_lateral(-50);
    const CANFrame frame = expect_frame(lateral_addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt16 frame sent to the lateral motor";
    EXPECT_EQ(static_cast<int16_t>(-50 * SWIVEL_MOTOR_RATIO), int16_at(frame, 0));
}

TEST_F(MicroscopeTest, SetHeightDoesNotMoveLateralMotor) {
    microscope.set_height(100);
    const CANFrame frame = expect_frame(lateral_addr, CommandID::TxInt16, 100);
    EXPECT_EQ(0, frame.can_dlc) << "set_height sent a frame to the lateral motor";
}
