#pragma once
#include "ExcavatorActuator.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"

class ExcavatorPayload {
private:
    WrappedCANBus can_bus;                  // concrete CAN implementation
    RoverCanMaster can_master;              // CAN logic handler
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload();
    ~ExcavatorPayload();
};