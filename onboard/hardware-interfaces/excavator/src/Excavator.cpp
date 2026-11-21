#include <Excavator.h>

#include <ExcavatorTypes.h>
#include <universalcan/CanTypes.h>

namespace {
constexpr CanAddress payload(DeviceID device) {
    return {GroupID::Payload, static_cast<uint8_t>(device)};
}
}  // namespace

Excavator::Excavator(RoverCanMaster &m)
    : m(m)
    , boom_tilt(payload(DeviceID::BoomTilt), m)
    , rock_bucket_tilt(payload(DeviceID::RockBucketTilt), m)
    , sand_bucket_tilt(payload(DeviceID::SandBucketTilt), m)
    , paver_magnet(DeviceID::PaverMagnet, m) {
      m.start_heartbeat(GroupID::Payload, DeviceID::BoomTilt);
    }


Excavator::~Excavator() {
  m.stop_heartbeat(GroupID::Payload, DeviceID::BoomTilt);
}

void Excavator::estop() { m.estop(GroupID::Payload, DeviceID::RockBucketTilt); }
