#include <ExcavatorPayload.h>

#include <ExcavatorTypes.h>
#include <universalcan/CanTypes.h>
ExcavatorPayload::ExcavatorPayload(RoverCanMaster &can_master)
    : can_master(can_master)
    , excavator_tilt(DeviceID::ExcavatorTilt, can_master)
    , bucket_tilt(DeviceID::BucketTilt, can_master)
    , teeth(DeviceID::Teeth, can_master)
    , paver_magnet(DeviceID::PaverMagnet, can_master) {}

void ExcavatorPayload::estop() {
    excavator_tilt.estop();
    bucket_tilt.estop();
    teeth.estop();
    paver_magnet.set_status(false);
}

ExcavatorPayload::~ExcavatorPayload() {}
