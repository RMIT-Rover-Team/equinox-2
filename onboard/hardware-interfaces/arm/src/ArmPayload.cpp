#include "ArmPayload.h"

ArmPayload::ArmPayload(GenericCan &can_bus)
  : can_bus(can_bus), can_master(can_bus, 0x0),
    motors(std::array<MyActuatorMotor, 6>{
        MyActuatorMotor(1, &can_bus),
        MyActuatorMotor(2, &can_bus),
        MyActuatorMotor(3, &can_bus),
        MyActuatorMotor(4, &can_bus),
        MyActuatorMotor(5, &can_bus),
        MyActuatorMotor(6, &can_bus)
    }),
    end_effector(can_master) {}

ArmPayload::~ArmPayload() {}

void ArmPayload::estop() {
    for (MyActuatorMotor& motor : motors) {
        motor.estop();
    }
    end_effector.estop();
}