#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include <cstring>
#include <iostream>
#include <format>
#include "../src/ExcavatorPayload.h"
#include "RoverCanMaster.h"
#include "GenericCan.h"

using ::testing::NiceMock;
using ::testing::Truly;
using ::testing::Exactly;
using ::testing::Return;
using ::testing::Eq;
using ::testing::_;


GroupId get_group_id(uint32_t header) {
    return (GroupId)((header >> 9) & 0b11);
}

uint8_t get_device(uint32_t header) {
    return (uint8_t)((header >> 4) & 0b11111);
}

CommandId get_command_id(uint32_t header) {
    return (CommandId)(header & 0b1111);
}

std::string get_command_name(uint16_t header) {
    CommandId cmd = get_command_id(header);
    
    switch (cmd) {
        case CommandId::PING: return "ping";
        case CommandId::ESTOP: return "estop";
        case CommandId::TXINT8: return "txint8";
        case CommandId::TXINT16: return "txint16";
        case CommandId::TXFLOAT: return "txfloat";
        case CommandId::TXDATA: return "txdata";
        default: return "unknown cmd";
    }
}

/// @brief Returns a string representation of the CAN data
/// @param cmd 
/// @param data 
/// @return 
std::string get_data(CommandId cmd, char data[CanDataLength]) {
    std::stringstream ss;

    if (cmd == CommandId::PING) return "";
    else if (cmd == CommandId::ESTOP) return "";
    else if (cmd == CommandId::TXINT8) {
        ss << ", data: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ", "
                         << data[4] << ", " << data[5] << ", " << data[6] << ", " << data[7];
        return ss.str();
    }
    else if (cmd == CommandId::TXINT16) {
        // reinterpret u8[8] to u16[4]
        int16_t data_16[CanDataLength/2];
        memcpy(data_16, data, CanDataLength);
        ss << ", data: " << data_16[0] << ", " << data_16[1] << ", " << data_16[2] << ", " << data_16[3];
        return ss.str();
    }
    else if (cmd == CommandId::TXFLOAT) {
        // reinterpret u8[8] to float[4]
        float data_f[CanDataLength/2];
        memcpy(data_f, data, CanDataLength);
        ss << ", data: " << data_f[0] << ", " << data_f[1] << ", " << data_f[2] << ", " << data_f[3];
        return ss.str();
    }
    else if (cmd == CommandId::TXDATA) {
        ss << ", data: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ", "
                         << data[4] << ", " << data[5] << ", " << data[6] << ", " << data[7];
        return ss.str();
    }
    else {
        ss << "UNKNOWN COMMAND ID";
        return ss.str();
    }
}


typedef struct {
    GroupId group_id;
    uint8_t device_id;
    CommandId cmd;
    char data[CanDataLength];
} CANData;

#define CANBufferLen 32

class VCANTest : public testing::Test {
protected:
    VCANTest()
     : can_bus("vcan0")
     , can_master(can_bus, 0x0)
     , excavator(can_master)
     , can_reader("vcan0") {}

    void SetUp() override {
        // clear out any messages previously sent
        read_from_vcan();
        i = 0;
        memset(msgs, 0, sizeof(msgs));
    }

    void read_from_vcan() {
        while (can_reader.available()) {
            // ensure buffer not full
            EXPECT_LT(i, CANBufferLen);
    
            CANFrame msg = can_reader.readMSG();

            CANData data {
                get_group_id(msg.can_id),
                get_device(msg.can_id),
                get_command_id(msg.can_id),
                {0}
            };

            memcpy(data.data, msg.data, CanDataLength);
    
            memcpy(&msgs[i], &data, sizeof(CANData));
            i++;
    
            std::printf("recieved %s to device %x%s \n", get_command_name(msg.can_id).c_str(), data.device_id, get_data(get_command_id(msg.can_id), data.data).c_str());
        }
    }

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    ExcavatorPayload excavator;
    WrappedCANBus can_reader;

    CANData msgs[CANBufferLen];
    uint8_t i;
};







TEST_F(VCANTest, EStop) {
    excavator.estop();

    read_from_vcan();

    // recieved 4 msgs
    EXPECT_EQ(i, 4);

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

    // recieved 3 msgs
    EXPECT_EQ(i, 3);

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

    // recieved 1 msg
    EXPECT_EQ(i, 1);

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

    // recieved 1 msg
    EXPECT_EQ(i, 4);

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
