#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus(CAN_BUS_NAME)                 // initialize first, order matters
    , can_master(can_bus, GroupId::PAYLOAD)
    , excavator_tilt(0x00, 0x00, can_master)
    , bucket_tilt(0x00, 0x01, can_master)
    , teeth(0x00, 0x02, can_master)
    , paver_magnet(0x03, can_master) {}

void ExcavatorPayload::estop() {
    excavator_tilt.estop();
    bucket_tilt.estop();
    teeth.estop();
}

ExcavatorPayload::~ExcavatorPayload() {}
