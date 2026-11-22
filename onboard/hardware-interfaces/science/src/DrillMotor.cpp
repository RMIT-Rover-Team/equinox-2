#include <DrillMotor.h>

DrillMotor::DrillMotor(CanAddress addr, RoverCanMaster &can_master) : m_(can_master), addr_(addr), status_(false) {}

void DrillMotor::set_rpm(int16_t rpm) {
  const int16_t msg[4] = {rpm, 0, 0, 0};
  m_.tx_int16(addr_.group, addr_.device, msg);
}

int16_t DrillMotor::get_rpm() {
  return rpm_;
}
