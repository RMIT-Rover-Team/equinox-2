#include <common/VelocityActuator.h>

VelocityActuator::VelocityActuator(CanAddress addr, RoverCanMaster &m)
    : addr(addr)
    , m(m) {}

void VelocityActuator::set_velocity(int16_t velocity) {
    this->velocity = velocity;
    const int16_t msg[4] = {velocity, 0, 0, 0};
    m.tx_int16(addr.group, addr.device, msg);
}

int16_t VelocityActuator::get_velocity() const {
    return velocity;
}
