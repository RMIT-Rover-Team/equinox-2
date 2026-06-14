#pragma once
#include "ExcavatorActuator.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"

class ExcavatorPayload {
private:
    SocketCanWrapper can_bus;                  // concrete CAN implementation
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload();
    ~ExcavatorPayload();
};