#include "VCanTest.h"



TEST_F(VCANTest, EStop) {
    gimbal.servo.estop();

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 1);

    EXPECT_EQ(msgs[0].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[0].cmd, CommandId::ESTOP);
    EXPECT_EQ(msgs[0].device_id, 0x0);
}

