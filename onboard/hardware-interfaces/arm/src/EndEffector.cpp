#include "EndEffector.h"


EndEffector::EndEffector(RoverCanMaster& can_master) : can_master(can_master) {}

EndEffector::~EndEffector() {}

void EndEffector::set_grip_velocity(int16_t target_velocity) {
    int16_t data[4] = {target_velocity};
    can_master.tx_int16(2, 0, data);
}

void EndEffector::set_poke_velocity(int16_t target_velocity) {
    int16_t data[4] = {target_velocity};
    can_master.tx_int16(2, 1, data);
}