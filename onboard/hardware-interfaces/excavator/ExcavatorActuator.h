#pragma once
#include <stdint.h>

class ExcavatorActuator
{
private:
    uint8_t id;
    double velocity;
public:
    ExcavatorActuator();
    ~ExcavatorActuator();
    double get_velocity();
    void set_velocity();
};
