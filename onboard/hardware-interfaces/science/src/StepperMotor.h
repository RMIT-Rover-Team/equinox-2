#ifndef EQUINOX_2_STEPPERMOTOR_H
#define EQUINOX_2_STEPPERMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class StepperMotor {
private:
    uint8_t device_id;
    RoverCanMaster &can_master;
    int16_t steps;
    int16_t last_command_steps;
public:
    StepperMotor(uint8_t device_id, RoverCanMaster &can_master);
    void set_steps(int16_t steps);
    int get_steps();
    void stop();
};


#endif //EQUINOX_2_STEPPERMOTOR_H
