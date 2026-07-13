/*
 * GenericMotor.h
 * Defines a generic motor abstraction for payload devices.
 * Used for motors that only need basic start/stop control.
 */

#ifndef EQUINOX_2_GENERICMOTOR_H
#define EQUINOX_2_GENERICMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>
#include <mutex>
#include <atomic>

class GenericMotor {
private:
    uint8_t device_id;
    RoverCanMaster can_master;
    int16_t current_rpm;
    int16_t target_rpm;
    bool target_changed;

    std::mutex motor_mutex;
    std::atomic<bool> stop_can_worker;
    std::thread can_worker_thread;

    void can_monitor_loop();
public:
    GenericMotor(uint8_t device_id, RoverCanMaster& can_master);
    ~GenericMotor();
    bool get_rpm() const;
    void set_rpm(int16_t rpm);
    void stop();
};


#endif //EQUINOX_2_GENERICMOTOR_H
