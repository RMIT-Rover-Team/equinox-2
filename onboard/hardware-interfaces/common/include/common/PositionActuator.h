#pragma once

#include <cstdint>
#include <universalcan/RoverCanMaster.h>

#include "CanAddress.h"

class PositionActuator {
public:
    PositionActuator(CanAddress addr, RoverCanMaster &m);

    void set_position(int16_t position);
    int16_t get_position() const;
private:
    CanAddress addr;
    RoverCanMaster &m;
    int16_t position = 0;
};
