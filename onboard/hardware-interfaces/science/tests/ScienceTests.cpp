#include <gtest/gtest.h>

#include <Science.h>
#include "VcanTest.h"

// CanTypes.h's unscoped `PayloadID` enum has an enumerator named `Science`,
// which hides the class name, so the class has to be named as `class Science`.
using SciencePayload = class Science;

struct ScienceTest : public VcanTest {};

TEST_F(ScienceTest, ConstructsAndDestroys) {
    EXPECT_NO_THROW({ SciencePayload science(test_can_master()); });
}

TEST_F(ScienceTest, SendsHeartbeatToBoard) {
    SciencePayload science(test_can_master());
    // The heartbeat worker pings every 500 ms.
    const CANFrame frame = expect_frame({GroupID::Payload, 0x00}, CommandID::Ping, 1500);
    EXPECT_NE(0u, frame.can_id) << "no heartbeat ping to the science board";
}

TEST_F(ScienceTest, HeaterIsWiredToItsAddress) {
    SciencePayload science(test_can_master());
    science.heater.set_heater_status(true);
    const CANFrame frame = expect_frame({GroupID::Payload, 0x04}, CommandID::TxInt8);
    ASSERT_EQ(8, frame.can_dlc);
    EXPECT_EQ(1, frame.data[0]);
}
