#pragma once
#include "RoverCanMaster.h"


class EndEffector {
private:
    RoverCanMaster &can_master;

public:
    EndEffector(RoverCanMaster& can_master);
    ~EndEffector();

    void set_grip_velocity(int16_t target_velocity);
    void set_poke_velocity(int16_t target_velocity);
    void estop();
};
