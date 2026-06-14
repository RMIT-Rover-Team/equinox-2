#pragma once
#include "ExcavatorActuator.h"

class ExcavatorPayload {
private:
    EQUCAN can_bus;                  // concrete CAN implementation
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload(GenericCan& can);
    ~ExcavatorPayload();
};