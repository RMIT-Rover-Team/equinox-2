/*
 * Defines the Microscope hardware abstraction.
 * Provides methods to set/get height and swivel angle
 * and reserves space for future distance sensor polling.
 */

#ifndef EQUINOX_2_MICROSCOPE_H
#define EQUINOX_2_MICROSCOPE_H

#include "../../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "StepperMotor.h"

constexpr double MICROSCOPE_HEIGHT_MOTOR_RATIO = 1.0;
constexpr double SWIVEL_MOTOR_RATIO = 1.0;

class Microscope {
private:
    // Two StepperMotors: Microscope class receives position (cm/deg) and
    // calls StepperMotor with number of steps
    StepperMotor height_motor;
    StepperMotor swivel_motor;
    double current_height;
    double target_height;
    double current_swivel;
    double target_swivel;
public:
    Microscope(uint8_t device_id, RoverCanMaster &can_master);
    ~Microscope();
    void setHeight(double height);
    void setSwivel(double swivel);
    double getCurrentHeight() const;
    double getTargetHeight() const;
    double getCurrentSwivel() const;
    double getTargetSwivel() const;
};


#endif //EQUINOX_2_MICROSCOPE_H
