#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class GimbalServo {
private:
    uint8_t device_id;
    int16_t tiltSpeed;
    int16_t panSpeed;
    RoverCanMaster &can_master; // can only have 1 master per bus, so use a ptr

public:
    GimbalServo(uint8_t device_id, RoverCanMaster &can_master);
    ~GimbalServo();
    void set_tilt_speed(int16_t target_tilt_speed);
    void set_pan_speed(int16_t target_pan_speed);
    void estop();
};