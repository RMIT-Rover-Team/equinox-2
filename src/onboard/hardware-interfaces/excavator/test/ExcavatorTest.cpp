#include "VCanTest.h"



TEST_F(VCANTest, EStop) {
    excavator.estop();

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 4);

    // excavator arm actuator
    EXPECT_EQ(msgs[0].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[0].cmd, CommandId::ESTOP);
    EXPECT_EQ(msgs[0].device_id, 0x0);

    // bucket actuator
    EXPECT_EQ(msgs[1].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[1].cmd, CommandId::ESTOP);
    EXPECT_EQ(msgs[1].device_id, 0x1);

    //teeth actuator
    EXPECT_EQ(msgs[2].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[2].cmd, CommandId::ESTOP);
    EXPECT_EQ(msgs[2].device_id, 0x2);

    // paver magnet
    EXPECT_EQ(msgs[3].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[3].cmd, CommandId::TXINT8);
    EXPECT_EQ(msgs[3].device_id, 0x3);
    EXPECT_EQ(msgs[3].data[0], 0);

    // internal states
    EXPECT_EQ(excavator.excavator_tilt.get_velocity(), 0);
    EXPECT_EQ(excavator.bucket_tilt.get_velocity(), 0);
    EXPECT_EQ(excavator.teeth.get_velocity(), 0);
    EXPECT_FALSE(excavator.paver_magnet.get_status());
}



TEST_F(VCANTest, MoveMotors) {
    excavator.excavator_tilt.set_velocity(-50);
    excavator.bucket_tilt.set_velocity(0);
    excavator.teeth.set_velocity(50);

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 3);

    // excavator arm actuator
    EXPECT_EQ(msgs[0].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[0].cmd, CommandId::TXINT16);
    EXPECT_EQ(msgs[0].device_id, 0x0);
    EXPECT_EQ(*(int16_t*)msgs[0].data, -50);

    // bucket actuator
    EXPECT_EQ(msgs[1].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[1].cmd, CommandId::TXINT16);
    EXPECT_EQ(msgs[1].device_id, 0x1);
    EXPECT_EQ(*(int16_t*)msgs[1].data, 0);

    //teeth actuator
    EXPECT_EQ(msgs[2].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[2].cmd, CommandId::TXINT16);
    EXPECT_EQ(msgs[2].device_id, 0x2);
    EXPECT_EQ(*(int16_t*)msgs[2].data, 50);

    // internal states
    EXPECT_EQ(excavator.excavator_tilt.get_velocity(), -50);
    EXPECT_EQ(excavator.bucket_tilt.get_velocity(), 0);
    EXPECT_EQ(excavator.teeth.get_velocity(), 50);
}



TEST_F(VCANTest, PaverMagnet) {
    excavator.paver_magnet.set_status(true);

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 1);

    // paver magnet
    EXPECT_EQ(msgs[0].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[0].cmd, CommandId::TXINT8);
    EXPECT_EQ(msgs[0].device_id, 0x3);
    EXPECT_EQ(msgs[0].data[0], 1);

    // internal state
    EXPECT_TRUE(excavator.paver_magnet.get_status());
}



TEST_F(VCANTest, Ping) {
    excavator.excavator_tilt.ping();
    excavator.bucket_tilt.ping();
    excavator.teeth.ping();
    excavator.paver_magnet.ping();

    read_from_vcan();
    EXPECT_EQ(msgs_recieved, 4);

    // excavator arm actuator
    EXPECT_EQ(msgs[0].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[0].cmd, CommandId::PING);
    EXPECT_EQ(msgs[0].device_id, 0x0);

    // bucket actuator
    EXPECT_EQ(msgs[1].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[1].cmd, CommandId::PING);
    EXPECT_EQ(msgs[1].device_id, 0x1);

    // teeth actuator
    EXPECT_EQ(msgs[2].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[2].cmd, CommandId::PING);
    EXPECT_EQ(msgs[2].device_id, 0x2);

    // paver magnet
    EXPECT_EQ(msgs[3].group_id, GroupId::PAYLOAD);
    EXPECT_EQ(msgs[3].cmd, CommandId::PING);
    EXPECT_EQ(msgs[3].device_id, 0x3);
}
