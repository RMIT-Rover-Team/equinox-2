/*
 * Drill.h
 * Defines the Drill hardware abstraction.
 * Stores drill height/status and exposes methods to set height,
 * start/stop the drill and retrieve the current stored state.
*/

#pragma once
#include <universalcan/RoverCanMaster.h>
#include <DrillMotor.h>
#include <common/CanAddress.h>
#include <common/VelocityActuator.h>

constexpr double DRILL_HEIGHT_MOTOR_RATIO = 1.0;
constexpr int16_t DRILL_RUNNING_RPM = 5000; // Confirm with engineering

struct Drill {
    VelocityActuator height_actuator_;
    DrillMotor drill_motor_;
    Drill(CanAddress height_addr, CanAddress drill_addr, RoverCanMaster& m);
    ~Drill();
};
