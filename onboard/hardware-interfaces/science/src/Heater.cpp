/*
 * Heater.cpp
 * Implements the Heater hardware interface.
 * set_temperature() stores the target temperature and sends it
 * to the heater device over the CAN bus as a float message.
 */
#include <Heater.h>

Heater::Heater(const CanAddress addr, RoverCanMaster& can_master)
  : m_(can_master)
  , addr_(addr) {}

void Heater::set_heater_status(bool status) {
  this->status_ = status;
  const int8_t msg[8] = {(int8_t)status, 0, 0, 0, 0, 0, 0, 0};
  m_.tx_int8(addr_.group, addr_.device, msg);
}

bool Heater::get_heater_status() const {
  return this->status_;
}
