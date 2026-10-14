#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "Gimbal.h"
#include "RoverCanMaster.h"
#include "SocketCanWrapper.h"


enum ThreadCommand {
    STOP
};


class CommsThread {
private:
    std::thread worker;
    // worker thread only

    /// if != target, need to send new packet
    int8_t last_sent_tilt_speed;
    int8_t last_sent_pan_position;

    // both worker and main thread
    
    std::mutex m_target;
    int8_t target_tilt_speed = 0;
    int8_t target_pan_position = 0;

    std::mutex m_should_stop;
    bool should_stop = false;

    // true if recieved a ping from the respective device
    std::atomic_bool excavator_tilt_alive{false};
    std::atomic_bool bucket_tilt_alive{false};

    std::mutex m_gimbal;

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    Gimbal gimbal;

    void run();
public:
    CommsThread(const char *can_interface);
    ~CommsThread();

    void start();
    void stop();

    bool excavator_alive();
    bool bucket_alive();

    void set_tilt_speed(int8_t speed);
    void set_pan_position(int8_t position);
    void estop();

    int16_t get_tilt_speed();
    int16_t get_pan_position();

    std::condition_variable cv;
};
