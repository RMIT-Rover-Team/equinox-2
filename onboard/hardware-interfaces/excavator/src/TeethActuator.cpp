#include "TeethActuator.h"

TeethActuator::TeethActuator(uint8_t device_id, RoverCanMaster &can_master) : ExcavatorActuator(device_id, can_master), latest_position(0.0) {}

TeethActuator::~TeethActuator() {}

// TODO: recieve encoded teeth position
double TeethActuator::get_teeth_pos() {
    return latest_position;
}