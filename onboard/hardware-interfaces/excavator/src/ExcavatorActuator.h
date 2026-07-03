#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "spdlog/spdlog.h"

class ExcavatorActuator {
protected:
    uint8_t device_id;
    int16_t velocity;
    RoverCanMaster &can_master; // can only have 1 master per bus, so use a ptr

public:
    // note that device id and motor id have been merged in the docs, discuss changing to 1 id param as well
    ExcavatorActuator(uint8_t device_id, RoverCanMaster &can_master);
    ~ExcavatorActuator();
    void set_velocity(int16_t target_velocity);
    void estop();
    void ping();
    void heartbeat();
};
