#pragma once
#include "ExcavatorActuator.h"
#include "TeethActuator.h"
#include "PaverMagnet.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"

#define CAN_BUS_NAME "can0"

enum DeviceId {
    EXCAVATOR_TILT = 0x0,
    BUCKET_TILT = 0x1,
    TEETH = 0x2,
    PAVER_MAGNET = 0x3
};

class ExcavatorPayload {
private:
    WrappedCANBus can_bus;                  // concrete CAN implementation
    RoverCanMaster can_master;              // CAN logic handler
    ExcavatorActuator excavator_tilt;
    ExcavatorActuator bucket_tilt;
    TeethActuator teeth;
    PaverMagnet paver_magnet;
public:
    ExcavatorPayload();
    ~ExcavatorPayload();
    void estop();
};