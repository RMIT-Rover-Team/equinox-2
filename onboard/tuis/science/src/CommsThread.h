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
    int16_t heater_temperature;
    double drill_height;
    bool drill_enabled;
    double microscope_height;
    double microscope_swivel;
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
    SciencePayload payload;

    void run();
public:
    CommsThread();
    ~CommsThread();

    void start();
    void stop();
    void estop();

    void get_mut_target_state(SciencePayloadState &state);    
};