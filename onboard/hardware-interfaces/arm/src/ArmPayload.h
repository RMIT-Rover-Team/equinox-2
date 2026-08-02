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
    ArmActuator motor1;
    ArmActuator motor2;
    ArmActuator motor3;
    ArmActuator motor4;
    ArmActuator motor5;
    ArmActuator motor6;
    EndEffector end_effector;

    ArmPayload(RoverCanMaster &can_master);
    ~ArmPayload();
    void estop();
};
