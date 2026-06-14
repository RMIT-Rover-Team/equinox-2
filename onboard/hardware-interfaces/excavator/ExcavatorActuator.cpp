#include "ExcavatorActuator.h" 

ExcavatorActuator::ExcavatorActuator(u_int8_t can_id, u_int8_t motor_id, GenericCan& can) : can_id(can_id), motor_id(motor_id), velocity(0.0), can_master(can, motor_id) {}

double ExcavatorActuator::get_velocity() {
    std::pair<ReceivedState, float> result = can_master.GetMotorSpeed(can_id, motor_id);
    velocity = result.second;

    if (result.first.error_flag) {
        // TODO
        // maybe set up an error buffer
    }

    if (result.first.uncallibrated_flag) {
        // TODO
        can_master.Calibrate(can_id, motor_id);
    }

    return velocity;
}

void ExcavatorActuator::set_velocity(double target_velocity) {
    velocity = target_velocity;
    ReceivedState result = can_master.SetMotorSpeed(can_id, motor_id, velocity);

    if (result.error_flag) {
        // TODO
        // maybe set up an error buffer
    }

    if (result.uncallibrated_flag) {
        // TODO
        can_master.Calibrate(can_id, motor_id);
    }
}

ExcavatorActuator::~ExcavatorActuator() {}
