#pragma once

#include <stdint.h>
#include "GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ArmActuator {
private:
    uint8_t device_id;
    uint8_t motor_id;
    int16_t velocity;
    RoverCanMaster &can_master;

public:
    ArmActuator(uint8_t device_id,
                uint8_t motor_id,
                RoverCanMaster &can_master);

    ~ArmActuator();

    void set_velocity(int16_t target_velocity);
};