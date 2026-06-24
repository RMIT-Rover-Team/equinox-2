#ifndef EQUINOX_2_HEATER_H
#define EQUINOX_2_HEATER_H

#include "../../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class Heater {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    bool status;
    double temperature;
public:
    Heater(uint8_t device_id, RoverCanMaster& can_master);
    void set_status(bool status);
    bool get_status();
    double get_temp();
    void tick_temperature();
};


#endif //EQUINOX_2_HEATER_H
