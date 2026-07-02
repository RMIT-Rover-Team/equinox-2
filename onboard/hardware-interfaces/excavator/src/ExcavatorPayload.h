#pragma once
#include "ExcavatorActuator.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"

#define CAN_BUS_NAME "can0"

class ExcavatorPayload {
private:
    WrappedCANBus can_bus;                  // concrete CAN implementation
    RoverCanMaster can_master;              // CAN logic handler
    ExcavatorActuator excavator_tilt;
    ExcavatorActuator bucket_tilt;
    ExcavatorActuator teeth;
public:
    ExcavatorPayload();
    ~ExcavatorPayload();
    void estop();
};