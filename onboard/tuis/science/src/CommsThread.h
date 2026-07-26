#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include "SciencePayload.h"
#include "RoverCanMaster.h"
#include "SocketCanWrapper.h"


enum ThreadCommand {
    STOP
};

typedef struct {
    int16_t heater_target_temperature;
    double drill_target_height;
    bool drill_target_enabled;
    double microscope_target_height;
    double microscope_target_swivel;
} SciencePayloadState;


class CommsThread {
private:
    std::thread worker;
    // worker thread only

    /// if != target, need to send new packet
    SciencePayloadState last_sent_state;

    // both worker and main thread
    
    std::mutex m_target_state;
    SciencePayloadState target_state;

    std::mutex m_should_stop;
    bool should_stop = false;

    // true if recieved a ping from the respective device
    std::atomic_bool excavator_tilt_alive{false};
    std::atomic_bool bucket_tilt_alive{false};

    std::mutex m_payload;

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    SciencePayload payload;

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
};