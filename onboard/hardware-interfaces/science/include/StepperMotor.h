/* StepperMotor.h
 * Defines a stepper motor abstraction for payload mechanisms.
 * Used by Drill and Microscope to send movement commands as step counts.
 */
 
#ifndef EQUINOX_2_STEPPERMOTOR_H
#define EQUINOX_2_STEPPERMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>

class StepperMotor {
private:
    uint8_t device_id;
    RoverCanMaster &can_master;
    int16_t current_steps;
    int16_t target_steps;
    bool target_changed;

    std::mutex motor_mutex;
    std::atomic<bool> stop_can_worker;
    std::thread can_worker_thread;

    void can_monitor_loop();
public:
    StepperMotor(uint8_t device_id, RoverCanMaster &can_master);
    ~StepperMotor();
    void set_steps(int16_t steps);
    int get_steps() const;
    void stop();
};


#endif //EQUINOX_2_STEPPERMOTOR_H
