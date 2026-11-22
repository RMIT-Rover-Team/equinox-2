/**
 * Drill.cpp
 * Implements the Drill hardware interface.
 * The Drill class controls two actuators:
 * 1. A height stepper motor for raising/lowering the drill.
 * 2. A generic motor for starting/stopping drill rotation.
 * Command are translated into CAN messages through StepperMotor and GenericMotor.
 */

#include <Drill.h>
#include <universalcan/CanTypes.h>

Drill::Drill(CanAddress height_addr, CanAddress drill_addr, RoverCanMaster& m)
    : height_actuator_(height_addr, m)
    , drill_motor_(drill_addr, m) {};

Drill::~Drill() {
}
