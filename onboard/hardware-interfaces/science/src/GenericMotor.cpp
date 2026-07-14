/*
 * GenericMotor.cpp
 * Implements a simple CAN-controlled motor interface.
 * start() sends a configured RPM command over CAN.
 * stop() sends zero RPM over CAN.
 * The class also tracks whether the motor is currently marked as running.
 */

#include "GenericMotor.h"

GenericMotor::GenericMotor(const uint8_t device_id, RoverCanMaster &can_master)
    : CanSender(device_id, can_master)
    , current_rpm(0)
    , target_rpm(0) {}

GenericMotor::~GenericMotor() {
    CanSender::~CanSender();
}

void GenericMotor::setCurrent(const int16_t current) {
    current_rpm = current;
}

bool GenericMotor::getRpm() const {
    return current_rpm;
}

void GenericMotor::setRpm(const int16_t target) {
    target_rpm = target;
    CanSender::setTarget(target);
}

void GenericMotor::stop() {
    target_rpm = 0;
    CanSender::setTarget(0);
}

