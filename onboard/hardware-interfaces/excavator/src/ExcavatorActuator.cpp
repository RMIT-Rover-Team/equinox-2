#include "ExcavatorActuator.h" 


ExcavatorActuator::ExcavatorActuator(uint8_t device_id, RoverCanMaster &can_master) : device_id(device_id), velocity(0.0), can_master(can_master) {}

void ExcavatorActuator::set_velocity(int16_t target_velocity) {
    spdlog::info("Set velocity of excavator actuator {0:x} to {1:d}", device_id, target_velocity);

    velocity = target_velocity;
    int16_t msg[4] = { target_velocity, 0, 0, 0 };
    can_master.tx_int16(GroupId::PAYLOAD, device_id, msg);
}

void ExcavatorActuator::estop() {
    spdlog::critical("ESTOP EXCAVATOR MOTOR {0:x}", device_id);

    velocity = 0;
    can_master.estop(GroupId::PAYLOAD, device_id);
}

void ExcavatorActuator::ping() {
    can_master.ping(GroupId::PAYLOAD, device_id);
}

void ExcavatorActuator::heartbeat() {
    return; // TODO: implement heartbeat
}

ExcavatorActuator::~ExcavatorActuator() {}
