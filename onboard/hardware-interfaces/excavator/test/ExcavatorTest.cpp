#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include "../src/ExcavatorPayload.h"
#include "RoverCanMaster.h"
#include "GenericCan.h"

using ::testing::NiceMock;
using ::testing::Truly;
using ::testing::Exactly;
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


class MockCanMaster : public RoverCanMaster {
public:
    MockCanMaster() : can_bus(MockCanWrapper()), RoverCanMaster(can_bus, 0x0) {}

    MOCK_METHOD(void, ping, (uint8_t group, uint8_t device));
    MOCK_METHOD(void, estop, (uint8_t group, uint8_t device));
    MOCK_METHOD(void, tx_int8, (uint8_t group, uint8_t device, int8_t[8]));
    MOCK_METHOD(void, tx_int16, (uint8_t group, uint8_t device, int16_t integers[4]));
    MOCK_METHOD(void, tx_float, (uint8_t group, uint8_t device, float integers[4]));
    MOCK_METHOD(void, tx_data, (uint8_t group, uint8_t device, uint8_t data[8]));
private:
    MockCanWrapper can_bus;
};


/// Returns a lambda predicate that matches for the given command id
std::function<bool(uint32_t)> IsCommand(int command_id) {
    return [command_id](uint32_t header) {
        std::printf("%x", header);
        return (header & 0x000f) == command_id;
    };
}




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

TEST(ExcavatorTests, ESTOP) {
    MockCanWrapper can_bus;
    // NiceMock<MockCanWrapper> can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);

    // set return value
    ON_CALL(can_bus, writeMSG(_, _, _)).WillByDefault(::testing::Return(0));

    // stop motors
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, 0x0, CommandId::ESTOP), _, _)).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, 0x1, CommandId::ESTOP), _, _)).Times(1);
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, 0x2, CommandId::ESTOP), _, _)).Times(1);
    // disable magnet
    EXPECT_CALL(can_bus, writeMSG(generate_header(GroupId::PAYLOAD, 0x3, CommandId::TXINT8), _, _)).Times(1);

    excavator.estop();
}

