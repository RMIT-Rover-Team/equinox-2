#pragma once
#include "ArmActuator.h"
#include "EndEffector.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"
#include <array>

class ArmPayload {
private:
    RoverCanMaster can_master;
public:
    std::array<ArmActuator, 6> motors;
    EndEffector end_effector;

    ArmPayload(RoverCanMaster &can_master);
    ~ArmPayload();
    void estop();
};
