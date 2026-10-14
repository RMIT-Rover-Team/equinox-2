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

TEST_F(VCANTest, MyActuatorStop) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    payload.motors.at(0).stop();

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);

    expect_msg(0, 1, 0, 0x81);
}


TEST_F(VCANTest, MyActuatorCalibrate) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    double vel = 10.0;
    payload.motors.at(0).calibrate();

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 2);

    expect_msg(0, 1, 0, 0x9b);
    expect_msg(1, 1, 0, 0x88);
}

TEST_F(VCANTest, MyActuatorSetPos) {
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

    int32_t angle;
    std::memcpy(&angle, msgs[0].data + sizeof(int32_t), sizeof(int32_t));
    EXPECT_EQ(angle, (int32_t)(pos*100));
}

TEST_F(VCANTest, MyActuatorSetVel) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    double vel = 10.0;
    payload.motors.at(0).setVelocity(vel);

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);

    expect_msg(0, 1, 0, 0xA2);

    int32_t angle;
    std::memcpy(&angle, msgs[0].data + sizeof(int32_t), sizeof(int32_t));
    EXPECT_EQ(angle, (int32_t)(vel*100));
}

TEST_F(VCANTest, MyActuatorGetPos) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    // hundredths of a degree
    int32_t angle = 50;

    std::exception_ptr thread_eptr = nullptr;
    // getposition blocks so need to send the response first
    // this is yucky
    std::thread send_response_thread([this, &thread_eptr, &angle] {
        try {
            // wait for interface to send position request
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            char response_data[CanDataLength] = {0};
            response_data[0] = 0x92;
            std::memcpy(response_data + 4, &angle, sizeof(int32_t));
        
            can_reader.writeMSG(0x0 + SingleMotorReplyIDOffset, response_data, sizeof(response_data));
            read_from_vcan(); // clear the message we sent from the reader
        } catch (...) {
            thread_eptr = std::current_exception();
        }
    });
    
    double pos = payload.motors.at(0).getPosition();
    // for some reason if i stream these separately they end up out of order
    std::cout << "angle recieved: " + std::to_string(pos) << std::endl;
    ASSERT_EQ((int32_t)(pos*100), 50);
    if (send_response_thread.joinable()) send_response_thread.join();

    // throw the exception in the main thread so gtest can handle it
    if (thread_eptr) std::rethrow_exception(thread_eptr);

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);

    expect_msg(0, 1, 0, 0x92);
}

TEST_F(VCANTest, EndEffectorGrip) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    int16_t vel = 10;
    payload.end_effector.set_grip_velocity(vel);

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);
    expect_msg(0, 2, 0, CommandId::TXINT16);

    int16_t recieved_vel;
    std::memcpy(&recieved_vel, msgs[0].data, sizeof(int16_t));
    EXPECT_EQ(recieved_vel, vel);
}

TEST_F(VCANTest, EndEffectorPoke) {
    auto expect_msg = [&](size_t index, char group_id, char device_id, char command) {
        EXPECT_EQ(msgs[index].group_id, group_id);
        EXPECT_EQ(msgs[index].device_id, device_id);
        EXPECT_EQ(msgs[index].cmd, command);
    };

    int16_t vel = 10;
    payload.end_effector.set_poke_velocity(vel);

    read_from_vcan();
    ASSERT_EQ(msgs_recieved, 1);
    expect_msg(0, 2, 1, CommandId::TXINT16);

    int16_t recieved_vel;
    std::memcpy(&recieved_vel, msgs[0].data, sizeof(int16_t));
    EXPECT_EQ(recieved_vel, vel);
}