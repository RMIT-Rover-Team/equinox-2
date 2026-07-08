#include "CommsThread.h"

std::mutex m_thread_queue;
std::mutex m_send_queue;
std::mutex m_recv_queue;

std::queue<ThreadCommand> thread_command_queue;
std::queue<std::pair<uint16_t, std::vector<char>>> send_queue;
std::queue<std::pair<uint16_t, std::vector<char>>> recv_queue;

// true if recieved a ping from the respective device
std::atomic_bool excavator_tilt_alive;
std::atomic_bool bucket_tilt_alive;


uint16_t generate_header(uint8_t group, uint8_t device, uint8_t command){
    uint16_t header = 0;

    header |= (group & 0x3) << 9;
    header |= (device & 0x1f) << 4;
    header |= (command & 0xf);

    return header;
}






CommsThread::CommsThread(GenericCan& can_bus) : can_bus(can_bus) {}
CommsThread::~CommsThread() {}



void CommsThread::start() {
    worker = std::thread(&CommsThread::run, this);
}

void CommsThread::stop() {
    std::lock_guard<std::mutex> lock(m_thread_queue);
    thread_command_queue.push(ThreadCommand::STOP);
    if (worker.joinable()) worker.join();
}



void CommsThread::start_heartbeat() {
    std::lock_guard<std::mutex> lock(m_thread_queue);
    thread_command_queue.push(ThreadCommand::START_HEARTBEAT);
}

void CommsThread::stop_heartbeat() {
    std::lock_guard<std::mutex> lock(m_thread_queue);
    thread_command_queue.push(ThreadCommand::STOP_HEARTBEAT);
}



bool CommsThread::excavator_alive() {
    return excavator_alive;
}
bool CommsThread::bucket_alive() {
    return bucket_alive;
}



void CommsThread::send_msg(u_int16_t header, char* data, u_int8_t len) {
    std::lock_guard<std::mutex> lock(m_send_queue);
    send_queue.push(std::pair<uint16_t, std::vector<char>>(header, std::vector(data, data + len)));
}




/// @brief  The loop run on the worker thread
void CommsThread::run() {
    while(true) {
        // handle commands from the main thread to this thread
        // exit if stop command given
        if (handle_thread_cmds()) break;

        handle_recv();
        handle_send();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
}


/// @brief Returns true if should exit main loop
bool CommsThread::handle_thread_cmds() {
    std::lock_guard<std::mutex> lock(m_thread_queue);

    while (!thread_command_queue.empty()) {
        switch (thread_command_queue.front()) {
            case ThreadCommand::START_HEARTBEAT:
                heartbeat_enabled = true;
                break;
            case ThreadCommand::STOP_HEARTBEAT:
                heartbeat_enabled = false;
                break;
            case ThreadCommand::STOP:
                return true;
        }

        thread_command_queue.pop();
    }

    return false;
}


void CommsThread::handle_recv() {
    // reset alive bools
    excavator_tilt_alive = false;
    bucket_tilt_alive = false;

    std::lock_guard<std::mutex> lock(m_recv_queue);

    while (can_bus.available()) {
        CANFrame frame = can_bus.readMSG();

        uint16_t header = frame.can_id;
        std::vector<char> data(frame.data, frame.data + frame.can_dlc);

        // handle recieve pings
        // if ping, set the matching ping bool
        if (heartbeat_enabled && header & 0xf == 0x0) {
            if ((header & 0x1f) >> 4 == DeviceId::EXCAVATOR_TILT) excavator_tilt_alive = true;
            else if ((header & 0x1f) >> 4 == DeviceId::BUCKET_TILT) bucket_tilt_alive = true;
        } else {
            std::pair<uint16_t, std::vector<char>> msg(header, data);
            recv_queue.emplace(msg);
        }
    }
}


void CommsThread::handle_send() {
    std::lock_guard<std::mutex> lock(m_send_queue);
    while (!send_queue.empty()) {
        can_bus.writeMSG(
            send_queue.front().first,
            send_queue.front().second.data(),
            send_queue.front().second.size()
        );

        send_queue.pop();
    }

    // send heartbeat msgs
    if (heartbeat_enabled) {
        can_bus.writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::EXCAVATOR_TILT, CommandId::PING), {}, 0);
        can_bus.writeMSG(generate_header(GroupId::PAYLOAD, DeviceId::BUCKET_TILT, CommandId::PING), {}, 0);
    }
}