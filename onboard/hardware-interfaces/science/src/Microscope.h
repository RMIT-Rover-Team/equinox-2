#ifndef EQUINOX_2_MICROSCOPE_H
#define EQUINOX_2_MICROSCOPE_H

#include "../../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "StepperMotor.h"

constexpr double height_motor_ratio = 0.0;
constexpr double swivel_motor_ratio = 0.0;

class Microscope {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    // Two StepperMotors: Microscope class receives position (cm/deg) and
    // calls StepperMotor with number of steps
    StepperMotor height_motor;
    StepperMotor swivel_motor;
    double height;
    double swivel;
public:
    Microscope(uint8_t device_id, RoverCanMaster &can_master);
    ~Microscope();
    void set_height(double height);
    void set_swivel(double swivel);
    double get_height();
    double get_swivel();
    void tick_distance_sensor();
};


#endif //EQUINOX_2_MICROSCOPE_H
