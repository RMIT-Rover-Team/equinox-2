#include "ArmPayload.h"

ArmPayload::ArmPayload(RoverCanMaster &can_master)
  : can_master(can_master),
    motor1(2, 0, can_master),
    motor2(2, 1, can_master),
    motor3(2, 2, can_master),
    motor4(2, 3, can_master),
    motor5(2, 4, can_master),
    motor6(2, 5, can_master),
    end_effector(2, 6, can_master) {}

ArmPayload::~ArmPayload() {}