/*
 * GenericMotor.cpp
 * Implements a simple CAN-controlled motor interface.
 * start() sends a configured RPM command over CAN.
 * stop() sends zero RPM over CAN.
 * The class also tracks whether the motor is currently marked as running.
 */

#include "GenericMotor.h"

GenericMotor::GenericMotor(const uint8_t device_id, RoverCanMaster &can_master)
    : CanDevice(device_id, can_master)
    , current_rpm(0)
    , target_rpm(0) {}

void GenericMotor::updateCurrent(const int16_t current) {
    current_rpm = current;
}

int16_t GenericMotor::getCurrentRpm() const {
    return current_rpm;
}

int16_t GenericMotor::getTargetRpm() const {
    return target_rpm;
}

void GenericMotor::setRpm(const int16_t target) {
    target_rpm = target;
    CanDevice::setTarget(target);
}

void GenericMotor::stop() {
    target_rpm = 0;
    CanDevice::setTarget(0);
}

