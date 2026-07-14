#include "CanSender.h"

CanSender::CanSender(uint8_t device_id, RoverCanMaster & can_master)
    : device_id(device_id)
    , can_master(&can_master)
{
    can_worker_thread = std::thread(&CanSender::canMonitorLoop, this);
    can_heartbeat_thread = std::thread(&CanSender::canHeartbeatLoop, this);
}

CanSender::~CanSender() {

}

void CanSender::setTarget(int16_t target) {

}

bool CanSender::isAlive() {
    return is_alive;
}

void CanSender::canMonitorLoop() {

}

void CanSender::canHeartbeatLoop() {

}

void CanSender::logFailedHeartbeat() {

}