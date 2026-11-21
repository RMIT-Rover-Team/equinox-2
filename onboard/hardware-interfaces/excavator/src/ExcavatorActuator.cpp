#include "ExcavatorActuator.h"

#include <universalcan/CanTypes.h>
#include "spdlog/spdlog.h"

ExcavatorActuator::ExcavatorActuator(uint8_t device_id, RoverCanMaster &can_master)
: device_id(device_id),
  velocity(0.0),
  can_master(can_master) {}
ExcavatorActuator::~ExcavatorActuator() {}

int16_t ExcavatorActuator::get_velocity() {
    return velocity;
}

void ExcavatorActuator::set_velocity(int16_t target_velocity) {
    spdlog::info("Set velocity of excavator actuator {0:x} to {1:d}", device_id, target_velocity);

    velocity = target_velocity;
    int16_t msg[4] = { target_velocity, 0, 0, 0 };
    can_master.tx_int16(GroupID::Payload, device_id, msg);
}

void ExcavatorActuator::estop() {
    spdlog::critical("ESTOP EXCAVATOR MOTOR {0:x}", device_id);

    velocity = 0;
    can_master.estop(GroupID::Payload, device_id);
}

void ExcavatorActuator::ping() {
    can_master.ping(GroupID::Payload, device_id);
}

void ExcavatorActuator::start_heartbeat() {
  can_master.start_heartbeat(GroupID::Payload, 0);
}
