/*
 * StepperMotor.cpp
 * Implements a CAN-controlled stepper motor interface.
 * set_steps() stores the most recent step command and sends it over CAN.
 * stop() clears the most recent command locally.
 */
 
#include "StepperMotor.h"

StepperMotor::StepperMotor(const uint8_t device_id, RoverCanMaster &can_master)
    : CanSender(device_id, can_master)
    , current_steps(0)
    , target_steps(0)
{
}

StepperMotor::~StepperMotor() {
    CanSender::~CanSender();
}

void StepperMotor::setCurrent(int16_t current) {
    current_steps = current;
}

void StepperMotor::setTargetSteps(const int16_t steps) {
    this->target_steps = steps;
    CanSender::setTarget(steps);
}

int16_t StepperMotor::getTargetSteps() const {
    return target_steps;
}

int16_t StepperMotor::getCurrentSteps() const {
    return current_steps;
}

void StepperMotor::stop() {
    target_steps = current_steps;
    CanSender::setTarget(current_steps);
}

