#include "ExcavatorActuator.h"
#include "GenericCan.h"
#include "EQUCAN.h"

class ExcavatorPayload {
private:
    EQUCAN can_bus;
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload() : can_bus(EQUCAN()), actuator1(1, 1, can_bus), actuator2(1, 2, can_bus) {}
    ~ExcavatorPayload();
};

ExcavatorPayload::~ExcavatorPayload() {}
