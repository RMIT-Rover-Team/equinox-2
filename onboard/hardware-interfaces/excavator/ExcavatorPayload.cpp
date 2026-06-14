#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus("can0")                      // initialize first, order matters
    , can_master(can_bus, 0x00)            // placeholder ID until Electrical confirms
    , actuator1(0x01, 0x01, &can_master)       // placeholder IDs until Electrical confirms
    , actuator2(0x01, 0x02, &can_master) {}

ExcavatorPayload::~ExcavatorPayload() {}
