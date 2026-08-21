#include "CanDevice.h"

CanDevice::CanDevice(const uint8_t device_id, RoverCanMaster & can_master)
    : device_id(device_id)
    , can_master(&can_master)
    , target_value(0)
    , current_value(0)
    , target_changed(false)
    , stop_can_worker(false)
    , stop_heartbeat_thread(false)
    , is_alive(true)
{
    can_worker_thread = std::thread(&CanDevice::canMonitorLoop, this);
    can_heartbeat_thread = std::thread(&CanDevice::canHeartbeatLoop, this);
    // stop_heartbeat_thread = true;
}

CanDevice::~CanDevice() {
    stop_can_worker = true;
    stop_heartbeat_thread = true;

    if (can_worker_thread.joinable()) {
        can_worker_thread.join();
    }

    if (can_heartbeat_thread.joinable()) {
        can_heartbeat_thread.join();
    }
}

void CanDevice::setTarget(int16_t target) {
    std::lock_guard<std::mutex> lock(target_mutex);
    target_value = target;
    target_changed = true;
}

bool CanDevice::isAlive() const {
    return is_alive;
}

void CanDevice::canMonitorLoop() {
    while (!stop_can_worker) {
        int16_t local_target = 0;
        bool should_update = false;

        {
            std::lock_guard<std::mutex> lock(target_mutex);
            if (target_changed) {
                local_target = target_value;
                should_update = true;
                target_changed = false;
            }
        }

        if (should_update) {
            int16_t message[4] = {local_target, 0, 0, 0};
            can_master->tx_int16(GroupId::PAYLOAD, device_id, message);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void CanDevice::canHeartbeatLoop() {
    while (!stop_heartbeat_thread) {
        can_master->ping(GroupId::PAYLOAD, device_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CanDevice::logFailedHeartbeat() const {
    std::cerr << "CanSender: Heartbeat not received for device_id ";
    std::cerr << device_id << std::endl;
}