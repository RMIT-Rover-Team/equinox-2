#pragma once
#include "common/DualModeActuator.h"
#include "PaverMagnet.h"
#include "universalcan/RoverCanMaster.h"

class Excavator {
private:
    RoverCanMaster &can_master;
public:
    ExcavatorActuator boom_tilt;
    ExcavatorActuator rock_bucket_tilt;
    ExcavatorActuator sand_bucket_tilt;
    PaverMagnet paver_magnet;

    Excavator(RoverCanMaster &can_master);
    ~Excavator();
    void estop();
};
