/*
 * Drill.h
 * Defines the Drill hardware abstraction.
 * Stores drill height/status and exposes methods to set height,
 * start/stop the drill and retrieve the current stored state.
*/

#ifndef EQUINOX_2_DRILL_H
#define EQUINOX_2_DRILL_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "StepperMotor.h"
#include "GenericMotor.h"

constexpr double HEIGHT_MOTOR_RATIO = 0.0;
constexpr int16_t DRILL_RUNNING_RPM = 5000; // Confirm with engineering

class Drill {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    StepperMotor height_motor;
    GenericMotor drill_motor;
    double drill_height;
    int16_t drill_current_rpm;
public:
    Drill(uint8_t device_id, RoverCanMaster &can_master);
    ~Drill();
    void setHeight(double height);
    void start(int16_t rpm);
    void start();
    void stop();
    double getCurrentHeight() const;
    double getStatus() const;
};


#endif //EQUINOX_2_DRILL_H
