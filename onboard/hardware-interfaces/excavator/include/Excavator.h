#pragma once
#include "common/VelocityActuator.h"
#include "PaverMagnet.h"
#include "universalcan/RoverCanMaster.h"

class Excavator {
private:
    RoverCanMaster &m;
public:
    VelocityActuator boom_tilt;
    VelocityActuator rock_bucket_tilt;
    VelocityActuator sand_bucket_tilt;
    PaverMagnet paver_magnet;

    Excavator(RoverCanMaster &can_master);
    ~Excavator();
    void estop();
};
