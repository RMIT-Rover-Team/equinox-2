#include <gtest/gtest.h>

#include <DrillMotor.h>
#include "VcanTest.h"

struct DrillMotorTest : public VcanTest {
    static constexpr CanAddress addr{GroupID::Payload, 0x03};
    DrillMotor motor{addr, test_can_master()};
};

TEST_F(DrillMotorTest, RpmInitialisesZero) {
    EXPECT_EQ(0, motor.get_rpm());
}

TEST_F(DrillMotorTest, SetRpmStoresValue) {
    motor.set_rpm(1200);
    EXPECT_EQ(1200, motor.get_rpm());
    motor.set_rpm(-1200);
    EXPECT_EQ(-1200, motor.get_rpm());
}

TEST_F(DrillMotorTest, SetRpmSendsTxInt16WithRpmInSlotZero) {
    motor.set_rpm(1200);
    const CANFrame frame = expect_frame(addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt16 frame sent to the drill motor";
    EXPECT_EQ(1200, int16_at(frame, 0));
    EXPECT_EQ(0, int16_at(frame, 1));
    EXPECT_EQ(0, int16_at(frame, 2));
    EXPECT_EQ(0, int16_at(frame, 3));
}
