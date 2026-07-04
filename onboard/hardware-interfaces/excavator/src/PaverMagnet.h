#pragma once
#include <stdint.h>
#include "GenericCan.h"
#include "RoverCanMaster.h"
#include "spdlog/spdlog.h"

class PaverMagnet {
private:
    uint8_t device_id;
    RoverCanMaster &can_master;
    bool status;
public:
    PaverMagnet(uint8_t device_id, RoverCanMaster &can_master);
    ~PaverMagnet();
    bool get_status();
    void set_status(bool status);
};
