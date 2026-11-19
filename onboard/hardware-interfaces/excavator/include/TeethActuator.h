#pragma once
#include "ExcavatorActuator.h"

class TeethActuator : public ExcavatorActuator {
private:
    double latest_position;
public:
    TeethActuator(uint8_t device_id, RoverCanMaster &can_master);
    ~TeethActuator();

    double get_teeth_pos();
};