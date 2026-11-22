#pragma once

#include <cstdint>
#include <universalcan/RoverCanMaster.h>

#include "CanAddress.h"

class StepperActuator {
public:
    StepperActuator(CanAddress addr, RoverCanMaster &m);

    void set_steps(int16_t position);
private:
    CanAddress addr;
    RoverCanMaster &m;
    int16_t position = 0;
};
