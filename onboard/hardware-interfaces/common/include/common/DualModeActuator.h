#pragma once

#include <cstdint>
#include <universalcan/RoverCanMaster.h>

#include "CanAddress.h"
#include "PositionActuator.h"
#include "VelocityActuator.h"

class DualModeActuator {
public:
    DualModeActuator(CanAddress velocity_addr,
                     CanAddress position_addr,
                     RoverCanMaster &m);

    void set_velocity(int16_t value);
    int16_t get_velocity() const;

    void set_position(int16_t value);
    int16_t get_position() const;
private:
    VelocityActuator velocity;
    PositionActuator position;
};
