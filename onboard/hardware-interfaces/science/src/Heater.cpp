/*
 * Heater.cpp
 * Implements the Heater hardware interface.
 * set_temperature() stores the target temperature and sends it
 * to the heater device over the CAN bus as a float message.
 */
#include "Heater.h"

Heater::Heater(uint8_t device_id, RoverCanMaster& can_master)
    : device_id(device_id)
    , can_master(can_master) {}

void Heater::set_temperature(float target_temperature) {
    this->target_temperature = target_temperature;
    float values[4] = {target_temperature, 0.0, 0.0, 0.0};
    can_master.tx_float(GroupId::PAYLOAD, device_id, values);
}

double Heater::get_temp() {
    return target_temperature;
}

void Heater::tick_temperature() {

}