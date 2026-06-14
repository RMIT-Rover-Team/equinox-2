#include "ExcavatorActuator.h"

class ExcavatorPayload {
private:
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload() : actuator1(1), actuator2(2) {}
    ~ExcavatorPayload();
};

ExcavatorPayload::~ExcavatorPayload() {}
