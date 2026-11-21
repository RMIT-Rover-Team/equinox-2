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
    : height_motor(0x01, can_master)
    , drill_motor(0x02, can_master)
    , current_height(0)
    , target_height(0)
    , current_rpm(0)
    , target_rpm(0) {};

Drill::~Drill() {
}

void Drill::set_height(double height) {
    this->target_height = height;
    height_motor.set_target_steps(height * DRILL_HEIGHT_MOTOR_RATIO);
}

void Drill::start(int16_t rpm) {
    this->target_rpm = rpm;
    drill_motor.set_rpm(rpm);
}

void Drill::start() {
    this->start(DRILL_RUNNING_RPM);
}

void Drill::stop() {
    this->target_rpm = 0;
    this->start(0);
}

double Drill::get_current_height() const {
    return height_motor.get_current_steps() / DRILL_HEIGHT_MOTOR_RATIO;
}

int16_t Drill::get_current_rpm() const {
    return drill_motor.get_current_rpm();
}

double Drill::get_target_height() const {
    return this->target_height;
}

int16_t Drill::get_target_rpm() const {
    return this->target_rpm;
}
