#include "SciencePayload.h"
#include <gtest/gtest.h>

TEST(DrillTest, SetHeight) {
    #define CAN_BUS_NAME = "can0";
    WrappedCANBus can_bus(CAN_BUS_NAME);
    RoverCanMaster can_master(can_bus, GroupId::PAYLOAD);
    Drill drill(0x02, can_master);
}