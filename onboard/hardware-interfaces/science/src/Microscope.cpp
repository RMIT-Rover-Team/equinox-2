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
    , swivel_motor(0x02, can_master)
    , current_height(0)
    , current_swivel(0) {};

Microscope::~Microscope() {
}

void Microscope::setHeight(double height) {
    height_motor.setTargetSteps(height * MICROSCOPE_HEIGHT_MOTOR_RATIO);
}

void Microscope::setSwivel(double swivel) {
    swivel_motor.setTargetSteps(swivel * SWIVEL_MOTOR_RATIO);
}

double Microscope::getCurrentHeight() const {
    return height_motor.getCurrentSteps() / MICROSCOPE_HEIGHT_MOTOR_RATIO;
}

double Microscope::getTargetHeight() const {
    return target_height;
}

double Microscope::getCurrentSwivel() const {
    return swivel_motor.getCurrentSteps() / SWIVEL_MOTOR_RATIO;
}

double Microscope::getTargetSwivel() const {
    return target_swivel;
}
