#include <common/StepperActuator.h>

StepperActuator::StepperActuator(CanAddress addr, RoverCanMaster &m)
    : addr(addr)
    , m(m) {}

void StepperActuator::set_steps(int16_t position) {
    this->position = position;
    const int16_t msg[4] = {position, 0, 0, 0};
    m.tx_int16(addr.group, addr.device, msg);
}
