#include "Gimbal.h"



Gimbal::Gimbal(RoverCanMaster& can_master) : can_master(can_master) {}
Gimbal::~Gimbal() {}

void Gimbal::estop() {
    spdlog::critical("ESTOP GIMBAL SERVO {0:x}", device_id);
    tilt_speed = 0.0;
    can_master.estop(GroupId::ONBOARD, device_id);
}

void Gimbal::set_tilt_speed(int8_t target_tilt_speed) {
    set_tilt_and_pan(target_tilt_speed, pan_position);
}

void Gimbal::set_pan_position(int8_t target_pan_position) {
    set_tilt_and_pan(tilt_speed, target_pan_position);
}

void Gimbal::set_tilt_and_pan(int8_t target_tilt_speed, int8_t target_pan_position) {
    spdlog::critical("Set gimbal servo {0:x} tilt speed to {1:d} and pan position to {2:d}", device_id, target_tilt_speed, target_pan_position);
    
    tilt_speed = target_tilt_speed;
    pan_position = target_pan_position;
    
    int8_t msg[8] = { tilt_speed, pan_position, 0, 0, 0, 0, 0, 0 };
    can_master.tx_int8(GroupId::ONBOARD, device_id, msg);
}

int8_t Gimbal::get_tilt_speed() {
    return tilt_speed;
}

int8_t Gimbal::get_pan_position() {
    return pan_position;
}
