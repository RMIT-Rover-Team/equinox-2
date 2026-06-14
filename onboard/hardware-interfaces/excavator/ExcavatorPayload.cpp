#include "ExcavatorPayload.h"

// #include "ExcavatorActuator.h"
// #include "GenericCan.h"
// #include "EQUCAN.h"
#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus()                      // ← initialize first, order matters
    , actuator1(0x01, can_bus)       // placeholder until Electrical confirms
    , actuator2(0x02, can_bus) {}

ExcavatorPayload::~ExcavatorPayload() {}

// class ExcavatorPayload {
// private:
//     EQUCAN can_bus;
//     ExcavatorActuator actuator1;
//     ExcavatorActuator actuator2;
// public:
//     // ExcavatorPayload::ExcavatorPayload(GenericCan& can)
//     // : actuator1(0x01, can_bus)   // placeholder until Electrical confirms
//     // , actuator2(0x02, can_bus) {}
//     // ~ExcavatorPayload();
// };
