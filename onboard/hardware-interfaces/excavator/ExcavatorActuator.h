#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
private:
    double velocity;
    RoverCanMaster can_master;

public:
    ExcavatorActuator(uint8_t id, GenericCan& can);
    ~ExcavatorActuator();
    double get_velocity();
    void set_velocity(double target_velocity);
};
