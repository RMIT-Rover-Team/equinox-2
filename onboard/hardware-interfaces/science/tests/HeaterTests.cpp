#include <gtest/gtest.h>

#include <Heater.h>
#include "VcanTest.h"

struct HeaterTest : public VcanTest {
    static constexpr CanAddress addr{GroupID::Payload, 0x04};
    Heater heater{addr, test_can_master()};
};

TEST_F(HeaterTest, StatusInitialisesOff) {
    EXPECT_FALSE(heater.get_heater_status());
}

TEST_F(HeaterTest, SetStatusStoresValue) {
    heater.set_heater_status(true);
    EXPECT_TRUE(heater.get_heater_status());
    heater.set_heater_status(false);
    EXPECT_FALSE(heater.get_heater_status());
}

TEST_F(HeaterTest, TurningOnSendsTxInt8One) {
    heater.set_heater_status(true);
    const CANFrame frame = expect_frame(addr, CommandID::TxInt8);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt8 frame sent to the heater";
    EXPECT_EQ(1, frame.data[0]);
}

TEST_F(HeaterTest, TurningOffSendsTxInt8Zero) {
    heater.set_heater_status(false);
    const CANFrame frame = expect_frame(addr, CommandID::TxInt8);
    ASSERT_EQ(8, frame.can_dlc) << "no TxInt8 frame sent to the heater";
    EXPECT_EQ(0, frame.data[0]);
}
