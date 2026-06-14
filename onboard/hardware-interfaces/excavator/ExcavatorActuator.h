#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
private:
    uint8_t can_id;
    uint8_t motor_id;
    double velocity;
    RoverCanMaster can_master; // Owns the CAN logic

public:
    ExcavatorActuator(uint8_t can_id, uint8_t motor_id, GenericCan& can);
    ~ExcavatorActuator();
    double get_velocity();
    void set_velocity(double target_velocity);
};
