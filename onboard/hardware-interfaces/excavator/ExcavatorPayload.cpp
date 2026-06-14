#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus()                      // initialize first, order matters
    , actuator1(0x01, 0x01, can_bus)       // placeholder until Electrical confirms
    , actuator2(0x01, 0x02, can_bus) {}

ExcavatorPayload::~ExcavatorPayload() {}
