#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "spdlog/spdlog.h"

class GimbalServo {
private:
    uint8_t device_id;
    int8_t tilt_speed;
    int8_t pan_position;
    RoverCanMaster &can_master; // can only have 1 master per bus, so use a ptr

public:
    GimbalServo(uint8_t device_id, RoverCanMaster &can_master);
    ~GimbalServo();
    void set_gimbal_movement(int8_t target_tilt_speed, int8_t target_pan_position);
    void estop();
};