#pragma once
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <atomic>
#include "ExcavatorPayload.h"
#include "GenericCan.h"
#include "CommandUtils.h"
#include "RoverCanMaster.h"


enum ThreadCommand {
    START_HEARTBEAT,
    STOP_HEARTBEAT,
    STOP
};



class CommsThread {
private:
    GenericCan& can_bus;
    bool heartbeat_enabled;
    // true if recieved a ping from the respective device
    std::atomic_bool excavator_tilt_alive{false};
    std::atomic_bool bucket_tilt_alive{false};

    std::mutex m_thread_queue;
    std::mutex m_send_queue;
    std::mutex m_recv_queue;

    std::queue<ThreadCommand> thread_command_queue;
    std::queue<std::pair<uint16_t, std::vector<char>>> send_queue;
    std::queue<std::pair<uint16_t, std::vector<char>>> recv_queue;

    std::thread worker;

    void run();
public:
    CommsThread(GenericCan& can_bus);
    ~CommsThread();

    void start();
    void stop();

    void start_heartbeat();
    void stop_heartbeat();

    bool excavator_alive();
    bool bucket_alive();

    void send_msg(u_int16_t header, char* data, u_int8_t len);
};
