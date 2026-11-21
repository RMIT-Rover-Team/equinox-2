#include <common/DualModeActuator.h>

DualModeActuator::DualModeActuator(CanAddress velocity_addr,
                                   CanAddress position_addr,
                                   RoverCanMaster &m)
    : velocity(velocity_addr, m)
    , position(position_addr, m) {}

void DualModeActuator::set_velocity(int16_t value) { velocity.set_velocity(value); }
int16_t DualModeActuator::get_velocity() const { return velocity.get_velocity(); }

void DualModeActuator::set_position(int16_t value) { position.set_position(value); }
int16_t DualModeActuator::get_position() const { return position.get_position(); }
