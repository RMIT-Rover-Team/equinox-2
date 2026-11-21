#include <Excavator.h>

#include <ExcavatorTypes.h>
#include <universalcan/CanTypes.h>
Excavator::Excavator(RoverCanMaster &can_master)
    : can_master(can_master)
    , boom_tilt(DeviceID::BoomTilt, can_master)
    , rock_bucket_tilt(DeviceID::RockBucketTilt, can_master)
    , sand_bucket_tilt(DeviceID::SandBucketTilt, can_master)
    , paver_magnet(DeviceID::PaverMagnet, can_master) {}

void Excavator::estop() {
  boom_tilt.estop();
  rock_bucket_tilt.estop();
  sand_bucket_tilt.estop();
  paver_magnet.estop();
}

Excavator::~Excavator() {}
