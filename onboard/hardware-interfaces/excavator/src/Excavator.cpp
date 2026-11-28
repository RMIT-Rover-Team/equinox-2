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
    m.handle_br_int16 = &Excavator::handle_position;

    m.start_heartbeat(GroupID::Payload, DeviceID::BoomTilt);
    m.start_listen();
}

Excavator::~Excavator() {
    m.stop_listen();      // joins the listen thread, so no handler is mid-call
    instance = nullptr;
    m.stop_heartbeat(GroupID::Payload, DeviceID::BoomTilt);
}

void Excavator::estop() { m.estop(GroupID::Payload, DeviceID::RockBucketTilt); }

void Excavator::handle_position(uint8_t group_id, uint8_t device_id,
                                std::span<const int16_t, 4> values) {
    Excavator *self = instance;
    if (self == nullptr || group_id != GroupID::Payload || device_id >= self->positions.size()) {
        return;
    }
    self->positions[device_id] = values[0];
}

int16_t Excavator::get_position(uint8_t device_id) const {
    return device_id < positions.size() ? positions[device_id].load() : 0;
}
