#include "ExcavatorActuator.h" 

/* 
 * Returns a 5 bit integer with the first 3 bits being the device id and the last 2 being the motor id
 */
uint8_t format_device_motor_id(uint8_t device_id, uint8_t motor_id) {
    return ((device_id & 0b00000111) << 2) | (motor_id & 0b00000011);
}

ExcavatorActuator::ExcavatorActuator(uint8_t device_id, uint8_t motor_id, RoverCanMaster &can_master) : device_id(device_id), motor_id(motor_id), velocity(0.0), can_master(can_master) {}

void ExcavatorActuator::set_velocity(int16_t target_velocity) {
    spdlog::critical("Set velocity of excavator actuator {0:x} to {0:d}", motor_id, target_velocity);

    velocity = target_velocity;
    int16_t msg[4] = { target_velocity, 0, 0, 0 };
    can_master.tx_int16(GroupId::PAYLOAD, format_device_motor_id(device_id, motor_id), msg);
}

void ExcavatorActuator::estop() {
    spdlog::critical("ESTOP EXCAVATOR MOTOR {0:x}", motor_id);

    velocity = 0;
    can_master.estop(GroupId::PAYLOAD, format_device_motor_id(device_id, motor_id));
}

ExcavatorActuator::~ExcavatorActuator() {}
