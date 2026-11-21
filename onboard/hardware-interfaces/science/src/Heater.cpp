/*
 * Heater.cpp
 * Implements the Heater hardware interface.
 * set_temperature() stores the target temperature and sends it
 * to the heater device over the CAN bus as a float message.
 */
#include "Heater.h"

Heater::Heater(const uint8_t device_id, RoverCanMaster& can_master)
    : CanDevice(device_id, can_master)
    , target_temperature(0)
    , current_temperature(0) {}

void Heater::set_target_temperature(const int16_t target) {
    this->target_temperature = target;
    CanDevice::set_target(target);
}

int16_t Heater::get_current_temperature() const {
    return current_temperature;
}

void Heater::update_current(const int16_t current) {
    current_temperature = current;
}

int16_t Heater::get_target_temperature() const {
    return target_temperature;
}
