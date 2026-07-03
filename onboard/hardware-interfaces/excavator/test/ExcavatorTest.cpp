#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdint.h>
#include "../src/ExcavatorPayload.h"
#include "RoverCanMaster.h"
#include "GenericCan.h"

using ::testing::NiceMock;
using ::testing::Truly;
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

bool IsEstop(uint32_t header) { return (header & 0x000f) == 0; }

TEST(ExcavatorTests, ESTOP) {
    NiceMock<MockCanWrapper> can_bus;
    RoverCanMaster can_master = RoverCanMaster(can_bus, 0x0);
    ExcavatorPayload excavator = ExcavatorPayload(can_master);

    excavator.estop();

    EXPECT_CALL(can_bus, writeMSG(Truly(IsEstop), _, _))
        .Times(3);
}