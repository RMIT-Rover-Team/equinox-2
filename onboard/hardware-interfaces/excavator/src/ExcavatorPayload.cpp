#include "ExcavatorPayload.h"

ExcavatorPayload::ExcavatorPayload()
    : can_bus(CAN_BUS_NAME)                 // initialize first, order matters
    , can_master(can_bus, GroupId::PAYLOAD)
    , excavator_tilt(DeviceId::EXCAVATOR_TILT, can_master)
    , bucket_tilt(DeviceId::BUCKET_TILT, can_master)
    , teeth(DeviceId::TEETH, can_master)
    , paver_magnet(DeviceId::PAVER_MAGNET, can_master) {}

void ExcavatorPayload::estop() {
    excavator_tilt.estop();
    bucket_tilt.estop();
    teeth.estop();
}

ExcavatorPayload::~ExcavatorPayload() {}
