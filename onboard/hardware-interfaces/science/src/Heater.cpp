/*
 * Heater.cpp
 * Implements the Heater hardware interface.
 * set_temperature() stores the target temperature and sends it
 * to the heater device over the CAN bus as a float message.
 */
#include "Heater.h"

Heater::Heater(const uint8_t device_id, RoverCanMaster& can_master)
    : CanSender(device_id, can_master)
    , target_temperature(0)
    , current_temperature(0) {}

Heater::~Heater() {
    CanSender::~CanSender();
}

void Heater::setTargetTemperature(const int16_t target) {
    this->target_temperature = target;
    CanSender::setTarget(target);
}

int16_t Heater::getCurrentTemperature() const {
    return current_temperature;
}

void Heater::setCurrent(const int16_t current) {
    current_temperature = current;
}

int16_t Heater::getTargetTemperature() const {
    return target_temperature;
}
