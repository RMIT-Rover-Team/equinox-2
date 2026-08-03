#include "ArmPayload.h"

ArmPayload::ArmPayload(RoverCanMaster &can_master)
  : can_master(can_master),
    motors(std::array<ArmActuator, 6>{
        ArmActuator(0, can_master),
        ArmActuator(1, can_master),
        ArmActuator(2, can_master),
        ArmActuator(3, can_master),
        ArmActuator(4, can_master),
        ArmActuator(5, can_master)
    }),
    end_effector(can_master) {}

ArmPayload::~ArmPayload() {}