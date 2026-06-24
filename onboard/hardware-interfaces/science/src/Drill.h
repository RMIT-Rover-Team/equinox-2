#ifndef EQUINOX_2_DRILL_H
#define EQUINOX_2_DRILL_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "StepperMotor.h"
#include "GenericMotor.h"

constexpr double height_motor_ratio = 0.0;

class Drill {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    StepperMotor height_motor;
    GenericMotor drill_motor;
    double drill_height;
    bool drill_status;
public:
    Drill(uint8_t device_id, RoverCanMaster &can_master);
    ~Drill();
    void set_height(double height);
    void run_drill(bool status);
    double get_height();
    double get_status();
};


#endif //EQUINOX_2_DRILL_H
