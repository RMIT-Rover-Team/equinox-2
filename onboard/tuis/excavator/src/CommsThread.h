#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "ExcavatorPayload.h"
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
    int16_t last_sent_excavator_velocity;
    int16_t last_sent_bucket_velocity;

    // both worker and main thread
    
    std::mutex m_target_velocity;
    int16_t excavator_target_velocity = 0;
    int16_t bucket_target_velocity = 0;

    std::mutex m_should_stop;
    bool should_stop = false;

    // true if recieved a ping from the respective device
    std::atomic_bool excavator_tilt_alive{false};
    std::atomic_bool bucket_tilt_alive{false};

    std::mutex m_excavator;

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    ExcavatorPayload excavator;

    void run();
public:
    CommsThread();
    ~CommsThread();

    void start();
    void stop();

    bool excavator_alive();
    bool bucket_alive();

    void set_excavator_velocity(int16_t velocity);
    void set_bucket_velocity(int16_t velocity);
    void estop();

    int16_t get_excavator_velocity();
    int16_t get_bucket_velocity();

    std::condition_variable cv;
};
