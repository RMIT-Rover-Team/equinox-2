#ifndef EQUINOX_2_CANSENDER_H
#define EQUINOX_2_CANSENDER_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>
#include <mutex>
#include <atomic>

// TODO: Implement listener to update current value
// TODO: Correctly implement heartbeat to listen for reply
class CanDevice {
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
    void canHeartbeatLoop();
    void logFailedHeartbeat() const;
protected:
    virtual void updateCurrent(int16_t current) = 0;
    virtual ~CanDevice();
public:
    CanDevice(uint8_t device_id, RoverCanMaster & can_master);
    bool isAlive() const;
    void setTarget(int16_t target);
    int16_t getCanTarget() const { return target_value; }
    int16_t getCanCurrent() const { return current_value; }
    int16_t getCanTargetChanged() const { return target_changed; }
};


#endif //EQUINOX_2_CANSENDER_H
