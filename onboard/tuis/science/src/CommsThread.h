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
    SciencePayloadState last_sent_state = {};

    // both worker and main thread

    std::mutex m_should_stop;
    bool should_stop = false;

    std::mutex m_payload;
    SciencePayload payload;

    void run();
public:
    CommsThread();
    ~CommsThread();

    void start();
    void stop();
    void estop();

    std::mutex m_target_state;
    SciencePayloadState target_state = {};
};