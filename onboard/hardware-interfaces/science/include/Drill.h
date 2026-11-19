/*
 * Drill.h
 * Defines the Drill hardware abstraction.
 * Stores drill height/status and exposes methods to set height,
 * start/stop the drill and retrieve the current stored state.
*/

#ifndef EQUINOX_2_DRILL_H
#define EQUINOX_2_DRILL_H

#include <universalcan/RoverCanMaster.h>
#include "StepperMotor.h"
#include "GenericMotor.h"

constexpr double DRILL_HEIGHT_MOTOR_RATIO = 1.0;
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
    void set_height(double height);
    void start(int16_t rpm);
    void start();
    void stop();
    double get_current_height() const;
    double get_target_height() const;
    int16_t get_current_rpm() const;
    int16_t get_target_rpm() const;
};


#endif //EQUINOX_2_DRILL_H
