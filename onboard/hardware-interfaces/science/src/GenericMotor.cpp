#include "GenericMotor.h"

GenericMotor::GenericMotor(uint8_t device_id, RoverCanMaster &can_master)
    : device_id()
    , can_master(can_master) {}

GenericMotor::~GenericMotor() {
}

bool GenericMotor::get_status() {
    return status;
}

void GenericMotor::start() {
    status = true;
    int16_t message[4] = {motor_rpm, 0, 0, 0};
    can_master.tx_int16(GroupId::PAYLOAD, device_id, message);
}

void GenericMotor::stop() {
    status = false;
    int16_t message[4] = {0, 0, 0, 0};
    can_master.tx_int16(GroupId::PAYLOAD, device_id, message);
}
