#include "Gimbal.h"



Gimbal::Gimbal(RoverCanMaster& can_master) : can_master(can_master), servo(0x00, can_master) {}
Gimbal::~Gimbal() {}

void Gimbal::estop() {
    servo.estop();
}

void Gimbal::set_tilt_speed(int8_t speed) {
    servo.set_gimbal_movement(speed, servo.get_pan_position());
}

void Gimbal::set_pan_position(int8_t position) {
    servo.set_gimbal_movement(servo.get_tilt_speed(), position);
}

void Gimbal::set_tilt_and_pan(int8_t target_tilt_speed, int8_t target_pan_position) {
    servo.set_gimbal_movement(target_tilt_speed, target_pan_position);
}
