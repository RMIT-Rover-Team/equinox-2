#ifndef EQUINOX_2_GENERICMOTOR_H
#define EQUINOX_2_GENERICMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

constexpr int16_t motor_rpm = 5000;

class GenericMotor {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    bool status;
public:
    GenericMotor(uint8_t device_id, RoverCanMaster& can_master);
    ~GenericMotor();
    bool get_status();
    void start();
    void stop();
};


#endif //EQUINOX_2_GENERICMOTOR_H
