#pragma once
#include "ArmActuator.h"
#include "EndEffector.h"
#include "MyActuatorMotor.h"
#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"
#include <array>

class ArmPayload {
private:
    GenericCan &can_bus;
    RoverCanMaster can_master;
public:
    std::array<MyActuatorMotor, 6> motors;
    EndEffector end_effector;

    ArmPayload(GenericCan &can_bus);
    ~ArmPayload();
    void estop();
};
