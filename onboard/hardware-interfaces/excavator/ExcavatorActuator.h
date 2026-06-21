#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
private:
    uint8_t device_id;
    uint8_t motor_id;
    int16_t velocity;
    RoverCanMaster &can_master; // can only have 1 master per bus, so use a ptr

public:
    ExcavatorActuator(uint8_t device_id, uint8_t motor_id, RoverCanMaster &can_master);
    ~ExcavatorActuator();
    void set_velocity(int16_t target_velocity);
};
