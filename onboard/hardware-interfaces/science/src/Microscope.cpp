/*
 * Microscope.h
 * Defines the Microscope hardware abstraction.
 * Controls microscope vertical height and swivel angle using two stepper motors.
 * Converts requested physical movement values into stepper motor commands.
 */

#include "Microscope.h"

// TODO: Update device_id when confirmed by engineering
Microscope::Microscope(uint8_t device_id, RoverCanMaster &can_master)
    : height_motor(0x01, can_master)
    , lateral_motor(0x02, can_master)
    , current_height(0)
    , current_lateral(0) {};

Microscope::~Microscope() {
}

void Microscope::set_height(double height) {
    height_motor.set_target_steps(height * MICROSCOPE_HEIGHT_MOTOR_RATIO);
}

void Microscope::set_lateral(double swivel) {
    lateral_motor.set_target_steps(swivel * SWIVEL_MOTOR_RATIO);
}

double Microscope::get_current_height() const {
    return height_motor.get_current_steps() / MICROSCOPE_HEIGHT_MOTOR_RATIO;
}

double Microscope::get_target_height() const {
    return target_height;
}

double Microscope::get_current_lateral() const {
    return lateral_motor.get_current_steps() / SWIVEL_MOTOR_RATIO;
}

double Microscope::get_target_lateral() const {
    return target_lateral;
}
