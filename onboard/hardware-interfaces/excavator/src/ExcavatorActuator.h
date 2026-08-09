#pragma once
#include <stdint.h>
#include "GenericCan.h"
#include "RoverCanMaster.h"
#include "spdlog/spdlog.h"

class ExcavatorActuator {
protected:
    uint8_t device_id;
    int16_t velocity;
    RoverCanMaster &can_master;

public:
    ExcavatorActuator(uint8_t device_id, RoverCanMaster &can_master);
    ~ExcavatorActuator();
    int16_t get_velocity();
    void set_velocity(int16_t target_velocity);
    void estop();
    void ping();
};
