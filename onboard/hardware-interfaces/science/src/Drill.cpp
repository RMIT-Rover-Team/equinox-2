/** 
 * Drill.cpp
 * Implements the Drill hardware interface.
 * The Drill class controls two actuators:
 * 1. A height stepper motor for raising/lowering the drill.
 * 2. A generic motor for starting/stopping drill rotation.
 * Command are translated into CAN messages through StepperMotor and GenericMotor.
 */

#include "Drill.h"

Drill::Drill(uint8_t device_id, RoverCanMaster& can_master)
    : device_id(device_id)
    , can_master(can_master)
    , height_motor(0x01, can_master)
    , drill_motor(0x02, can_master)
    , drill_height(0)
    , drill_current_rpm(0) {};

Drill::~Drill() {
}

void Drill::set_height(double height) {
    drill_height = height;
    int16_t steps = HEIGHT_MOTOR_RATIO * height;
    height_motor.set_steps(steps);
}

void Drill::run_drill(int16_t rpm) {
    drill_motor.set_rpm(rpm);
}

double Drill::get_height() const {
    return drill_height;
}

double Drill::get_status() const {
    return drill_current_rpm;
}
