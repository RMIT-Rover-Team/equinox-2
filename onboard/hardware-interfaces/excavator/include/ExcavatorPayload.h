#pragma once
#include "ExcavatorActuator.h"
#include "TeethActuator.h"
#include "PaverMagnet.h"
#include "universalcan/RoverCanMaster.h"

class ExcavatorPayload {
private:
    RoverCanMaster &can_master;
public:
    ExcavatorActuator excavator_tilt;
    ExcavatorActuator bucket_tilt;
    TeethActuator teeth;
    PaverMagnet paver_magnet;

    ExcavatorPayload(RoverCanMaster &can_master);
    ~ExcavatorPayload();
    void estop();
};
