#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include <cstring>
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

class MockCanWrapper : public GenericCan {
public:
    MockCanWrapper() : GenericCan() {}

    MOCK_METHOD(CANFrame, readMSG, (), (override));
    MOCK_METHOD(CANFrame, readMSGFrom, (uint32_t Id, uint32_t Mask), (override));
    MOCK_METHOD(CANFrame, readMSGFrom, (uint32_t Id, uint32_t Mask, uint32_t timeout_ms), (override));
    MOCK_METHOD(CANFrame, readReturnMSGFrom, (uint32_t Id, uint32_t Mask, uint32_t timeout_ms, uint32_t command_id), (override));
    MOCK_METHOD(int, writeMSG, (uint32_t IdAndFlags, const char* data, uint8_t length), (override));
    MOCK_METHOD(void, clearBuffer, (), (override));
    MOCK_METHOD(bool, available, (), (override));
    MOCK_METHOD(bool, availableFrom, (uint32_t Id, uint32_t Mask), (override));
};

// MockCanMaster removed - RoverCanMaster's methods aren't virtual, so
// MOCK_METHOD on them can never intercept calls made through a
// RoverCanMaster&. Mocking GenericCan (MockCanWrapper) is the correct
// interception point - RoverCanMaster runs for real in every test.

// class MockCanMaster : public RoverCanMaster {
// public:
//     MockCanMaster() : can_bus(MockCanWrapper()), RoverCanMaster(can_bus, 0x0) {}

//     MOCK_METHOD(void, ping, (uint8_t group, uint8_t device));
//     MOCK_METHOD(void, estop, (uint8_t group, uint8_t device));
//     MOCK_METHOD(void, tx_int8, (uint8_t group, uint8_t device, int8_t[8]));
//     MOCK_METHOD(void, tx_int16, (uint8_t group, uint8_t device, int16_t integers[4]));
//     MOCK_METHOD(void, tx_float, (uint8_t group, uint8_t device, float integers[4]));
//     MOCK_METHOD(void, tx_data, (uint8_t group, uint8_t device, uint8_t data[8]));
// private:
//     MockCanWrapper can_bus;
// };

uint16_t generate_header(uint8_t group, uint8_t device, uint8_t command){
    uint16_t header = 0;
    
    header |= (group & 0x3) << 9;
    header |= (device & 0x1f) << 4;
    header |= (command & 0xf);
    
    return header;
}

/*

11000000000 group
00111110000 device
00000001111 command

10000000001 1025
10000010001 1041
10000100001 1057

10000110010 1074

*/

::testing::Matcher<const char*> Int16PayloadEq(int16_t expected) {
    return Truly([expected](const char* data) {
        int16_t actual = 0;
        std::memcpy(&actual, data, sizeof(int16_t));
        if (actual != expected) return false;
        for (int i = 2; i < 8; i++) {
            if (data[i] != 0) return false;
        }
        return true;
    });
}

TEST(ExcavatorTests, ESTOP) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);

    // set return value
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(::testing::Return(0));

    // stop motors
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::ESTOP), _, _)).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::ESTOP), _, _)).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::TEETH, CommandId::ESTOP), _, _)).Times(1);

    // disable magnet
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::PAVER_MAGNET, CommandId::TXINT8), _, _)).Times(1);

    excavator.estop();
}

TEST(ExcavatorTests, EstopIsIdempotent) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);

    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(Return(0));

    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::ESTOP), _, _)).Times(2);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::ESTOP), _, _)).Times(2);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::TEETH, CommandId::ESTOP), _, _)).Times(2);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::PAVER_MAGNET, CommandId::TXINT8), _, _)).Times(2);

    excavator.estop();
    excavator.estop();

    EXPECT_EQ(excavator.excavator_tilt.get_velocity(), 0);
    EXPECT_EQ(excavator.bucket_tilt.get_velocity(), 0);
    EXPECT_EQ(excavator.teeth.get_velocity(), 0);
}

TEST(ExcavatorTests, MoveMotors) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);

    // set return value
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(::testing::Return(0));

    // start motors
    // EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::TXINT16), ::testing::StrEq("\x5"), _)).Times(1);
    // EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::TXINT16), ::testing::StrEq("\x5"), _)).Times(1);
    // EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::TEETH, CommandId::TXINT16), ::testing::StrEq("\x5"), _)).Times(1);

    // full payload (not just the first byte) and length are checked, StrEq
    // stops comparing at the first embedded null byte so it missed the padding
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::TXINT16), Int16PayloadEq(5), Eq(8))).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::TXINT16), Int16PayloadEq(5), Eq(8))).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::TEETH, CommandId::TXINT16), Int16PayloadEq(5), Eq(8))).Times(1);

    excavator.excavator_tilt.set_velocity(5);
    excavator.bucket_tilt.set_velocity(5);
    excavator.teeth.set_velocity(5);

    EXPECT_EQ(excavator.excavator_tilt.get_velocity(), 5);
    EXPECT_EQ(excavator.bucket_tilt.get_velocity(), 5);
    EXPECT_EQ(excavator.teeth.get_velocity(), 5);
}

TEST(ExcavatorTests, MoveMotorsHandlesNegativeAndBoundaryVelocities) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);
 
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(Return(0));
 
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::TXINT16), Int16PayloadEq(-500), Eq(8))).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::TXINT16), Int16PayloadEq(INT16_MAX), Eq(8))).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::TEETH, CommandId::TXINT16), Int16PayloadEq(INT16_MIN), Eq(8))).Times(1);
 
    excavator.excavator_tilt.set_velocity(-500);
    excavator.bucket_tilt.set_velocity(INT16_MAX);
    excavator.teeth.set_velocity(INT16_MIN);
}
 
TEST(ExcavatorTests, Ping) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);
 
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(Return(0));
 
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::PING), _, Eq(0))).Times(1);
 
    excavator.excavator_tilt.ping();
}
 
TEST(ExcavatorTests, PaverMagnetSetStatusRoundTrip) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);
 
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(Return(0));
 
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::PAVER_MAGNET, CommandId::TXINT8),
                                   Truly([](const char* data) { return data[0] == 1; }), _)).Times(1);
 
    excavator.paver_magnet.set_status(true);
 
    EXPECT_TRUE(excavator.paver_magnet.get_status());
}
 
TEST(ExcavatorTests, TeethActuatorDefaultPosition) {
    MockCanWrapper can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);
 
    // No CAN traffic expected just from construction.
    EXPECT_EQ(excavator.teeth.get_teeth_pos(), 0.0);
}

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

std::string get_data(CommandId cmd, char data[CanDataLength]) {
    if (cmd = CommandId::PING) return "";
    if (cmd = CommandId::ESTOP) return "";
    if (cmd = CommandId::TXINT8) return std::format("{}, {}, {}, {}, {}, {}, {}, {}", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
    if (cmd = CommandId::TXINT16) {
        // reinterpret u8[8] to u16[4]
        uint16_t data_16[CanDataLength/2];
        memcpy(data_16, data, CanDataLength);
        return std::format("{}, {}, {}, {}", data_16[0], data_16[1], data_16[2], data_16[3]);
    }
    if (cmd = CommandId::TXFLOAT) {
        // reinterpret u8[8] to float[4]
        float data_f[CanDataLength/2];
        memcpy(data_f, data, CanDataLength);
        return std::format("{}, {}, {}, {}", data_f[0], data_f[1], data_f[2], data_f[3]);
    }
    if (cmd = CommandId::TXDATA) return std::format("{}, {}, {}, {}, {}, {}, {}, {}", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

    return "UNKNOWN COMMAND ID";
}


typedef struct {
    GroupId group_id;
    uint8_t device_id;
    CommandId cmd;
    char data[8];
} CANData;

#define CANBufferLen 32

class VCANTest : public testing::Test {
protected:
    VCANTest()
     : can_bus("vcan0")
     , can_master(can_bus, 0x0)
     , excavator(can_master)
     , can_reader("vcan0") {}

    void read_from_vcan() {
        while (can_reader.available()) {
            // ensure buffer not full
            EXPECT_LT(i, CANBufferLen);
    
            CANFrame msg = can_reader.readMSG();
    
            memcpy(&msgs[i], &msg, sizeof(CANFrame));
            i++;
    
            std::printf("recieved %s to device %x, data: %s \n", get_command_name(msg.can_id).c_str(), get_device(msg.can_id), get_data(get_command_id(msg.can_id), msg.data).c_str());
        }
    }

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    ExcavatorPayload excavator;
    WrappedCANBus can_reader;

    CANFrame msgs[CANBufferLen];
    uint8_t i;
};


TEST_F(VCANTest, VCAN) {
    excavator.estop();

    read_from_vcan();

    // recieved 4 msgs
    EXPECT_EQ(i, 4);

    // excavator arm actuator
    EXPECT_EQ(get_group_id(msgs[0].can_id), GroupId::PAYLOAD);
    EXPECT_EQ(get_command_id(msgs[0].can_id), CommandId::ESTOP);
    EXPECT_EQ(get_device(msgs[0].can_id), 0x0);

    // bucket actuator
    EXPECT_EQ(get_group_id(msgs[1].can_id), GroupId::PAYLOAD);
    EXPECT_EQ(get_command_id(msgs[1].can_id), CommandId::ESTOP);
    EXPECT_EQ(get_device(msgs[1].can_id), 0x1);

    //teeth actuator
    EXPECT_EQ(get_group_id(msgs[2].can_id), GroupId::PAYLOAD);
    EXPECT_EQ(get_command_id(msgs[2].can_id), CommandId::ESTOP);
    EXPECT_EQ(get_device(msgs[2].can_id), 0x2);

    // paver magnet
    EXPECT_EQ(get_group_id(msgs[3].can_id), GroupId::PAYLOAD);
    EXPECT_EQ(get_command_id(msgs[3].can_id), CommandId::TXINT8);
    EXPECT_EQ(get_device(msgs[3].can_id), 0x3);
    EXPECT_EQ(msgs[3].data[0], 0);
}
