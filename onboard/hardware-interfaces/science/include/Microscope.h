/*
 * Defines the Microscope hardware abstraction.
 * Provides methods to set/get height and swivel angle
 * and reserves space for future distance sensor polling.
 */

#ifndef EQUINOX_2_MICROSCOPE_H
#define EQUINOX_2_MICROSCOPE_H

#include <universalcan/GenericCan.h>
#include <universalcan/RoverCanMaster.h>
#include "StepperMotor.h"

constexpr double MICROSCOPE_HEIGHT_MOTOR_RATIO = 1.0;
constexpr double SWIVEL_MOTOR_RATIO = 1.0;

class Microscope {
private:
    // Two StepperMotors: Microscope class receives position (cm/deg) and
    // calls StepperMotor with number of steps
    StepperMotor height_motor;
    StepperMotor lateral_motor;
    double current_height;
    double target_height;
    double current_lateral;
    double target_lateral;
public:
    Microscope(uint8_t device_id, RoverCanMaster &can_master);
    ~Microscope();
    void set_height(double height);
    void set_lateral(double swivel);
    double get_current_height() const;
    double get_target_height() const;
    double get_current_lateral() const;
    double get_target_lateral() const;
};


#endif //EQUINOX_2_MICROSCOPE_H
