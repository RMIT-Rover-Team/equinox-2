#pragma once
#include "ExcavatorActuator.h"
#include "TeethActuator.h"
#include "PaverMagnet.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"


enum DeviceId {
    EXCAVATOR_TILT = 0x0,
    BUCKET_TILT = 0x1,
    TEETH = 0x2,
    PAVER_MAGNET = 0x3
};

class ExcavatorPayload {
private:
    RoverCanMaster can_master;
    ExcavatorActuator excavator_tilt;
    ExcavatorActuator bucket_tilt;
    TeethActuator teeth;
    PaverMagnet paver_magnet;
public:
    ExcavatorPayload(RoverCanMaster &can_master);
    ~ExcavatorPayload();
    void estop();
};