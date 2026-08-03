#include "ArmActuator.h"


ArmActuator::ArmActuator(uint8_t motor_id, RoverCanMaster &can_master)
 : motor_id(motor_id), velocity(0), can_master(can_master) {}

ArmActuator::~ArmActuator() {}

/*
myactuator ids

send: 0010100xxxx
recv: 0100100xxxx

send 1 motor: 0x140 + ID (4 or 5 bits idk)
send multi motor: 0x280
reply: 0x240 + ID (4 or 5 bits idk)

*/
void ArmActuator::set_velocity(int16_t target_velocity) {
    
}

void ArmActuator::set_position(int16_t target_position) {
    
}

int32_t ArmActuator::get_encoded_position() {
    int32_t pos = 0;
    
    can_master.tx_int8(0b01, motor_id, )
}