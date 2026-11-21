#pragma once

#include <cstdint>
#include <universalcan/RoverCanMaster.h>

#include "CanAddress.h"

class VelocityActuator {
public:
    VelocityActuator(CanAddress addr, RoverCanMaster &m);

    void set_velocity(int16_t velocity);
    int16_t get_velocity() const;

private:
    CanAddress addr;
    RoverCanMaster &m;
    int16_t velocity = 0;
};
