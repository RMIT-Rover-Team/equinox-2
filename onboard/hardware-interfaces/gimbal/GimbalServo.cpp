#include "GimbalServo.h"

GimbalServo::GimbalServo(uint8_t device_id, RoverCanMaster &can_master) : device_id(device_id), can_master(can_master), tilt_speed(0.0), pan_position(0.0) {}

void GimbalServo::set_gimbal_movement(int8_t target_tilt_speed, int8_t target_pan_position) {
    spdlog::critical("Set gimbal servo {0:x} tilt speed to {1:d} and pan position to {2:d}", device_id, target_tilt_speed, target_pan_position);

    tilt_speed = target_tilt_speed;
    pan_position = target_pan_position;

    int8_t msg[8] = { tilt_speed, pan_position, 0, 0, 0, 0, 0, 0 };
    can_master.tx_int8(GroupId::ONBOARD, device_id, msg);
}

void GimbalServo::estop() {
    spdlog::critical("ESTOP GIMBAL SERVO {0:x}", device_id);

    tilt_speed = 0.0;
    pan_position = 0.0;
    
    can_master.estop(GroupId::ONBOARD, device_id);
}

GimbalServo::~GimbalServo() {}
