#include "Drill.h"

Drill::Drill(uint8_t device_id, RoverCanMaster& can_master)
    : device_id(device_id)
    , can_master(can_master)
    , height_motor(0x01, can_master)
    , drill_motor(0x02, can_master) {};

Drill::~Drill() {
}

void Drill::set_height(double height) {
    drill_height = height;
    int16_t steps = height_motor_ratio * height;
    height_motor.set_steps(steps);
}

void Drill::run_drill(bool status) {
    drill_status = status;

    if (status)
        drill_motor.start();
    if (!status)
        drill_motor.stop();
}

double Drill::get_height() {
    return drill_height;
}

double Drill::get_status() {
    return drill_status;
}
