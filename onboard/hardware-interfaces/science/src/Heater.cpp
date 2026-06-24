#include "Heater.h"

Heater::Heater(uint8_t device_id, RoverCanMaster& can_master)
    : device_id(device_id)
    , can_master(can_master) {}

void Heater::set_status(bool status) {
    this->status = status;
    // TODO: Need to communicate on CANBus to turn heater on/off
}

bool Heater::get_status() {
    return status;
}

double Heater::get_temp() {
    return temperature;
}

void Heater::tick_temperature() {

}