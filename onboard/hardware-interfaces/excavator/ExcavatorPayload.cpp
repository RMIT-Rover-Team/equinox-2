#include "ExcavatorActuator.h"

class ExcavatorPayload {
private:
    ExcavatorActuator actuator1;
    ExcavatorActuator actuator2;
public:
    ExcavatorPayload();
    ~ExcavatorPayload();
};

ExcavatorPayload::ExcavatorPayload() {}

ExcavatorPayload::~ExcavatorPayload() {}
