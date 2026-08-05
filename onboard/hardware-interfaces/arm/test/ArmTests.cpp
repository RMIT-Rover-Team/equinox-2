#include "VCanTest.h"



TEST_F(VCANTest, EStop) {
    payload.estop();

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

