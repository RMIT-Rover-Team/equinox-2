#include <gtest/gtest.h>

#include <Drill.h>
#include "VcanTest.h"

struct DrillTest : public VcanTest {
    static constexpr CanAddress height_addr{GroupID::Payload, 0x00};
    static constexpr CanAddress drill_addr{GroupID::Payload, 0x03};
    Drill drill{height_addr, drill_addr, test_can_master()};
};

TEST_F(DrillTest, HeightVelocityGoesToHeightActuator) {
    drill.height_actuator_.set_velocity(300);
    const CANFrame frame = expect_frame(height_addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt16 frame sent to the height actuator";
    EXPECT_EQ(300, int16_at(frame, 0));
    EXPECT_EQ(300, drill.height_actuator_.get_velocity());
}

TEST_F(DrillTest, RpmGoesToDrillMotor) {
    drill.drill_motor_.set_rpm(DRILL_RUNNING_RPM);
    const CANFrame frame = expect_frame(drill_addr, CommandID::TxInt16);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt16 frame sent to the drill motor";
    EXPECT_EQ(DRILL_RUNNING_RPM, int16_at(frame, 0));
}

TEST_F(DrillTest, HeightAndDrillUseSeparateAddresses) {
    drill.drill_motor_.set_rpm(100);
    const CANFrame frame = expect_frame(height_addr, CommandID::TxInt16, 100);
    EXPECT_EQ(0, frame.can_dlc) << "set_rpm sent a frame to the height actuator";
}
