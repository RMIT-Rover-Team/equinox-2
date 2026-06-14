#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
private:
    uint8_t can_id;
    uint8_t motor_id;
    double velocity;
    RoverCanMaster &can_master; // can only have 1 master per bus, so use a ptr

public:
    ExcavatorActuator(uint8_t can_id, uint8_t motor_id, RoverCanMaster &can_master);
    ~ExcavatorActuator();
    double get_velocity();
    void set_velocity(double target_velocity);
};
