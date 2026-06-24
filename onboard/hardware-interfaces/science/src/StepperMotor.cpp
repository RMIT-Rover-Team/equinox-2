#include "StepperMotor.h"

StepperMotor::StepperMotor(uint8_t device_id, RoverCanMaster &can_master)
    : device_id(device_id)
    , can_master(can_master) {}

void StepperMotor::set_steps(int16_t steps) {
    last_command_steps = steps;
    int16_t message[4] = {steps, 0, 0, 0};
    can_master.tx_int16(GroupId::PAYLOAD, device_id, message);
}

int StepperMotor::get_steps() {
}

void StepperMotor::stop() {
    last_command_steps = 0;
}