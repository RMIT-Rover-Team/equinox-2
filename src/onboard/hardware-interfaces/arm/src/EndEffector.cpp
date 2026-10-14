#include "EndEffector.h"


EndEffector::EndEffector(RoverCanMaster& can_master) : can_master(can_master) {}

EndEffector::~EndEffector() {}

void EndEffector::set_grip_velocity(int16_t target_velocity) {
    int16_t data[4] = {target_velocity};
    can_master.tx_int16(GroupId::PAYLOAD, 0, data);
}

void EndEffector::set_poke_velocity(int16_t target_velocity) {
    int16_t data[4] = {target_velocity};
    can_master.tx_int16(GroupId::PAYLOAD, 1, data);
}

void EndEffector::estop() {
    can_master.estop(GroupId::PAYLOAD, 0);
}