#include "VCanTest.h"



TEST_F(VCANTest, EStop) {
    gimbal.estop();

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 1);

    EXPECT_EQ(msgs[0].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[0].device_id, 0x0);
    EXPECT_EQ(msgs[0].cmd, CommandId::ESTOP);
}

TEST_F(VCANTest, EStopResetsTiltSpeed) {
    gimbal.set_tilt_and_pan(50, -30);
    gimbal.estop();

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 2);

    EXPECT_EQ(msgs[0].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[0].device_id, 0x0);
    EXPECT_EQ(msgs[0].cmd, CommandId::TXINT8);
    EXPECT_EQ((int8_t)msgs[0].data[0], 50);
    EXPECT_EQ((int8_t)msgs[0].data[1], -30);

    EXPECT_EQ(msgs[1].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[1].device_id, 0x0);
    EXPECT_EQ(msgs[1].cmd, CommandId::ESTOP);

    EXPECT_EQ(gimbal.get_tilt_speed(), 0);
    EXPECT_EQ(gimbal.get_pan_position(), -30);
}

TEST_F(VCANTest, TiltAndPanSavedBetweenMsgs) {
    gimbal.set_tilt_speed(50);
    gimbal.set_pan_position(-30);
    gimbal.set_tilt_speed(10);

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 3);

    EXPECT_EQ(msgs[0].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[0].device_id, 0x0);
    EXPECT_EQ(msgs[0].cmd, CommandId::TXINT8);
    EXPECT_EQ((int8_t)msgs[0].data[0], 50);
    EXPECT_EQ((int8_t)msgs[0].data[1], 0);

    EXPECT_EQ(msgs[1].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[1].device_id, 0x0);
    EXPECT_EQ(msgs[1].cmd, CommandId::TXINT8);
    EXPECT_EQ((int8_t)msgs[1].data[0], 50);
    EXPECT_EQ((int8_t)msgs[1].data[1], -30);

    EXPECT_EQ(msgs[2].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[2].device_id, 0x0);
    EXPECT_EQ(msgs[2].cmd, CommandId::TXINT8);
    EXPECT_EQ((int8_t)msgs[2].data[0], 10);
    EXPECT_EQ((int8_t)msgs[2].data[1], -30);

    EXPECT_EQ(gimbal.get_tilt_speed(), 10);
    EXPECT_EQ(gimbal.get_pan_position(), -30);
}

TEST_F(VCANTest, SetBothSendsOneMsg) {
    gimbal.set_tilt_and_pan(50, -30);

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 1);

    EXPECT_EQ(msgs[0].group_id, GroupId::ONBOARD);
    EXPECT_EQ(msgs[0].device_id, 0x0);
    EXPECT_EQ(msgs[0].cmd, CommandId::TXINT8);
    EXPECT_EQ((int8_t)msgs[0].data[0], 50);
    EXPECT_EQ((int8_t)msgs[0].data[1], -30);

    EXPECT_EQ(gimbal.get_tilt_speed(), 50);
    EXPECT_EQ(gimbal.get_pan_position(), -30);
}

