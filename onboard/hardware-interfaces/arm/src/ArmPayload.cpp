#include "ArmPayload.h"

ArmPayload::ArmPayload(RoverCanMaster &can_master)
  : can_master(can_master),
    motor1(1, 0, can_master),
    motor2(1, 1, can_master),
    motor3(1, 2, can_master),
    motor4(1, 3, can_master),
    motor5(1, 4, can_master),
    motor6(1, 5, can_master),
    end_effector(can_master) {}

ArmPayload::~ArmPayload() {}