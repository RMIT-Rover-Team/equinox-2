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

void GenericMotor::update_current(const int16_t current) {
    current_rpm = current;
}

int16_t GenericMotor::get_current_rpm() const {
    return current_rpm;
}

int16_t GenericMotor::get_target_rpm() const {
    return target_rpm;
}

void GenericMotor::set_rpm(const int16_t target) {
    target_rpm = target;
    CanDevice::set_target(target);
}

void GenericMotor::stop() {
    target_rpm = 0;
    CanDevice::set_target(0);
}

