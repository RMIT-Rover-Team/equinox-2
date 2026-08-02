#include "ArmActuator.h"


ArmActuator::ArmActuator(uint8_t device_id, uint8_t motor_id, RoverCanMaster &can_master)
: device_id(device_id), motor_id(motor_id), velocity(0), can_master(can_master) {}

ArmActuator::~ArmActuator() {}

void ArmActuator::set_velocity(int16_t target_velocity) {
    
}