#pragma once

#include "GimbalServo.h"
#include "GenericCan.h"

class Gimbal {
private:
    RoverCanMaster can_master;
    GimbalServo servo;

public:
    Gimbal(RoverCanMaster& can_master);
    ~Gimbal();
    void estop();
    void set_tilt_speed(int8_t speed);
    void set_pan_position(int8_t position);
    void set_tilt_and_pan(int8_t target_tilt_speed, int8_t target_pan_position);
};
