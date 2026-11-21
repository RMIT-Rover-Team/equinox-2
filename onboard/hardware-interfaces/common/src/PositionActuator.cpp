#include "PositionActuator.h"

PositionActuator::PositionActuator(CanAddress addr, RoverCanMaster &m)
    : addr(addr)
    , m(m) {}

void PositionActuator::set_position(int16_t position) {
    this->position = position;
    const int16_t msg[4] = {position, 0, 0, 0};
    m.tx_int16(addr.group, addr.device, msg);
}

int16_t PositionActuator::get_position() const {
    return position;
}
