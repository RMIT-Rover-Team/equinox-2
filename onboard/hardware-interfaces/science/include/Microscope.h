/*
 * Defines the Microscope hardware abstraction.
 * Provides methods to set/get height and swivel angle
 * and reserves space for future distance sensor polling.
 */

#pragma once

#include <universalcan/GenericCan.h>
#include <universalcan/RoverCanMaster.h>
#include <common/StepperActuator.h>

constexpr double MICROSCOPE_HEIGHT_MOTOR_RATIO = 1.0;
constexpr double SWIVEL_MOTOR_RATIO = 1.0;

class Microscope {
private:
    StepperActuator height_motor;
    StepperActuator lateral_motor;
public:
    Microscope(CanAddress height_addr, CanAddress lateral_addr, RoverCanMaster &can_master);
    ~Microscope();
    void set_height(double height);
    void set_lateral(double swivel);
};
