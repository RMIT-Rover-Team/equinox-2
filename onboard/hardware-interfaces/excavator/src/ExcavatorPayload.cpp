#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus(CAN_BUS_NAME)                 // initialize first, order matters
    , can_master(can_bus, GroupId::PAYLOAD)
    , actuator1(0x01, 0x01, can_master)    // placeholder IDs until Electrical confirms
    , actuator2(0x01, 0x02, can_master) {} // pass the reference itself, not its address

void ExcavatorPayload::estop() {
    actuator1.estop();
    actuator2.estop();
}

ExcavatorPayload::~ExcavatorPayload() {}
