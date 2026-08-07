#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "ArmPayload.h"
#include "RoverCanMaster.h"
#include "SocketCanWrapper.h"


enum ThreadCommand {
    STOP
};

typedef struct {
    std::array<double, 6> motor_positions;
    int16_t poke_velocity;
    int16_t grip_velocity;
} ArmPayloadState;


class CommsThread {
private:
    std::thread worker;
    // worker thread only

    /// if != target, need to send new packet
    ArmPayloadState last_sent_state = {};

    // both worker and main thread

    std::mutex m_should_stop;
    bool should_stop = false;

    std::mutex m_payload;
    ArmPayload payload;

    void run();
public:
    CommsThread(GenericCan &can_bus);
    ~CommsThread();

    void start();
    void stop();
    void estop();

    std::condition_variable cv;
    std::mutex m_target_state;
    ArmPayloadState target_state = {};
    std::array<double, 6> encoded_motor_positions;
};