#ifndef EQUINOX_2_CANSENDER_H
#define EQUINOX_2_CANSENDER_H

#include <universalcan/RoverCanMaster.h>
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
    bool alive;

    void can_monitor_loop();
    void can_heartbeat_loop();
    void log_failed_heartbeat() const;
protected:
    virtual void update_current(int16_t current) = 0;
    virtual ~CanDevice();
public:
    CanDevice(uint8_t device_id, RoverCanMaster & can_master);
    bool is_alive() const;
    void set_target(int16_t target);
    int16_t get_can_target() const { return target_value; }
    int16_t get_can_current() const { return current_value; }
    int16_t get_can_target_changed() const { return target_changed; }
};


#endif //EQUINOX_2_CANSENDER_H
