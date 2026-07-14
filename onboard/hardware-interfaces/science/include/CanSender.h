#ifndef EQUINOX_2_CANSENDER_H
#define EQUINOX_2_CANSENDER_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>
#include <mutex>
#include <atomic>

class CanSender {
private:
    uint8_t device_id;
    RoverCanMaster* can_master;
    int16_t target_value;
    int16_t current_value;
    bool target_changed;

    std::mutex target_mutex;
    std::atomic<bool> stop_can_worker;
    std::thread can_worker_thread;
    std::atomic<bool> stop_heartbeat_thread;
    std::thread can_heartbeat_thread;
    bool is_alive;

    void canMonitorLoop();
    void canHeartbeatLoop() const;
    void logFailedHeartbeat() const;
public:
    CanSender(uint8_t device_id, RoverCanMaster & can_master);
    ~CanSender();
    bool isAlive() const;
    void setTarget(int16_t target);
    virtual void setCurrent(int16_t current) = 0;
};


#endif //EQUINOX_2_CANSENDER_H
