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
    StepperMotor height_motor;
    GenericMotor drill_motor;
    double current_height;
    double target_height;
    int16_t current_rpm;
    int16_t target_rpm;
public:
    Drill(uint8_t device_id, RoverCanMaster &can_master);
    ~Drill();
    void setHeight(double height);
    void start(int16_t rpm);
    void start();
    void stop();
    double getCurrentHeight() const;
    double getTargetHeight() const;
    int16_t getCurrentRpm() const;
    int16_t getTargetRpm() const;
};


#endif //EQUINOX_2_DRILL_H
