#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator
{
private:
    uint8_t id;
    double velocity;
public:
    ExcavatorActuator(uint8_t id);
    ~ExcavatorActuator();
    double get_velocity();
    void set_velocity();
};
