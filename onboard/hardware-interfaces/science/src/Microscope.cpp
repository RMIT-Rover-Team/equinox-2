/*
 * Microscope.h
 * Defines the Microscope hardware abstraction.
 * Controls microscope vertical height and swivel angle using two stepper motors.
 * Converts requested physical movement values into stepper motor commands. 
 */
 
#include "Microscope.h"

// TODO: Update device_id when confirmed by engineering
Microscope::Microscope(uint8_t device_id, RoverCanMaster &can_master)
    : device_id(device_id)
    , can_master(can_master)
    , height_motor(0x01, can_master)
    , swivel_motor(0x02, can_master) {};

Microscope::~Microscope() {
}

void Microscope::setHeight(double height) {
    this->height = height;

    int16_t steps = HEIGHT_MOTOR_RATIO * (height - this->height);
    height_motor.setTargetSteps(steps);
}

void Microscope::setSwivel(double swivel) {
    this->swivel = swivel;

    while (swivel >= 180) {
        swivel -= 360;
    }
    while (swivel <= -180) {
        swivel += 360;
    }

    int16_t steps = SWIVEL_MOTOR_RATIO * (swivel - this->swivel);
    swivel_motor.setTargetSteps(steps);
}

double Microscope::getHeight() const {
    return height;
}

double Microscope::getSwivel() const {
    return swivel;
}
