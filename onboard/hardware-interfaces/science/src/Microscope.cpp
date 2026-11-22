/*
 * Microscope.h
 * Defines the Microscope hardware abstraction.
 * Controls microscope vertical height and swivel angle using two stepper motors.
 * Converts requested physical movement values into stepper motor commands.
 */

#include "Microscope.h"
#include <universalcan/CanTypes.h>

Microscope::Microscope(CanAddress height_addr, CanAddress lateral_addr, RoverCanMaster &can_master)
    : height_motor(height_addr, can_master)
    , lateral_motor(lateral_addr, can_master) {};

Microscope::~Microscope() {
}

void Microscope::set_height(double height) {
    height_motor.set_steps(height * MICROSCOPE_HEIGHT_MOTOR_RATIO);
}

void Microscope::set_lateral(double swivel) {
    lateral_motor.set_steps(swivel * SWIVEL_MOTOR_RATIO);
}
