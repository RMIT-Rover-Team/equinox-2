#pragma once

#include <stdint.h>
#include "GenericCan.h"
#include "RoverCanMaster.h"
#include "spdlog/spdlog.h"

// Controls the gimbal servo via CAN bus
// Handles tilt speed and pan position for camera movement
class Gimbal {
private:
    RoverCanMaster &can_master;
    uint8_t device_id = 0;

    int8_t tilt_speed = 0;
    int8_t pan_position = 0;

public:
    Gimbal(RoverCanMaster& can_master);
    ~Gimbal();
    void estop();
    void set_tilt_speed(int8_t speed);
    void set_pan_position(int8_t position);
    void set_tilt_and_pan(int8_t target_tilt_speed, int8_t target_pan_position);
    int8_t get_tilt_speed();
    int8_t get_pan_position();
};
