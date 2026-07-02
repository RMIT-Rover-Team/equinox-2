#pragma once
#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "spdlog/spdlog.h"

class PaverMagnet {
private:
    uint8_t device_id : 5;
    RoverCanMaster &can_master;
    bool status;
public:
    PaverMagnet(uint8_t device_id, RoverCanMaster &can_master);
    ~PaverMagnet();
    void set_status(bool status);
    bool get_status();
};
