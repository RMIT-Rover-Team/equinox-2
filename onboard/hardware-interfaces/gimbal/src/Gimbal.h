#pragma once

#include "GimbalServo.h"
#include "GenericCan.h"

class Gimbal {
private:
    RoverCanMaster can_master;

public:
    Gimbal(RoverCanMaster& can_master);
    ~Gimbal();
    GimbalServo servo;
};
