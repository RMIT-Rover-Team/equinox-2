#include "Microscope.h"

// TODO: Update device_id when confirmed by engineering
Microscope::Microscope(uint8_t device_id, RoverCanMaster &can_master)
    : device_id(device_id)
    , can_master(can_master)
    , height_motor(0x01, can_master)
    , swivel_motor(0x02, can_master) {};

Microscope::~Microscope() {
}

void Microscope::set_height(double height) {
    this->height = height;

    int16_t steps = height_motor_ratio * (height - this->height);
    height_motor.set_steps(steps);
}

void Microscope::set_swivel(double swivel) {
    this->swivel = swivel;

    while (swivel >= 180) {
        swivel -= 360;
    }
    while (swivel <= -180) {
        swivel += 360;
    }

    int16_t steps = swivel_motor_ratio * (swivel - this->swivel);
    swivel_motor.set_steps(steps);
}

double Microscope::get_height() {
    return height;
}

double Microscope::get_swivel() {
    return swivel;
}

void Microscope::tick_distance_sensor() {
}
