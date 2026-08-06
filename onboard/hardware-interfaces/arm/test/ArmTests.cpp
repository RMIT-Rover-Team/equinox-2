#include "VCanTest.h"



TEST_F(VCANTest, EStop) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    payload.estop();

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 7);

    // estop all 6 myactuators
    for (int i = 0; i < 6; ++i) {
        expect_msg(i, 1, i, 0x80);
    }
    
    // end effector
    expect_msg(6, 2, 0, CommandId::ESTOP);
}

TEST_F(VCANTest, SetMotorPos) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    double pos = 10.0;
    payload.motors.at(0).setPosition(pos);

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);

    expect_msg(0, 1, 0, 0xA4);

    // [0] is speed limit, [1] is the position
    int32_t angle[CanDataLength/4] = {0};
    std::memcpy(&angle, msgs[0].data, CanDataLength);
    EXPECT_EQ(angle[1], (int32_t)(pos*100));
}

