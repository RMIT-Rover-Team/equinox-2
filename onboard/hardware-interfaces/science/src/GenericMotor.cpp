/*
 * GenericMotor.cpp
 * Implements a simple CAN-controlled motor interface.
 * start() sends a configured RPM command over CAN.
 * stop() sends zero RPM over CAN.
 * The class also tracks whether the motor is currently marked as running.
 */

#include "GenericMotor.h"

GenericMotor::GenericMotor(uint8_t device_id, RoverCanMaster &can_master)
    : device_id()
    , can_master(can_master)
    , current_rpm(0)
    , target_rpm(0)
    , target_changed(false)
    , stop_can_worker(false)
{
    can_worker_thread = std::thread(&GenericMotor::can_monitor_loop, this);
}

GenericMotor::~GenericMotor() {
    stop_can_worker = true;
    if (can_worker_thread.joinable())
        can_worker_thread.join();
}

bool GenericMotor::get_rpm() const {
    return current_rpm;
}

void GenericMotor::set_rpm(int16_t rpm) {
    target_rpm = rpm > 0 ? rpm : 0;
    target_changed = true;
}

void GenericMotor::stop() {
    set_rpm(0);
}

void GenericMotor::can_monitor_loop() {
    while (!stop_can_worker) {
        int16_t local_target_rpm = 0;
        bool should_update = false;

        {
            std::lock_guard<std::mutex> lock(motor_mutex);
            if (target_changed) {
                local_target_rpm = target_rpm;
                should_update = true;
                target_changed = false;
            }
        }

        if (should_update) {
            int16_t message[4] = {local_target_rpm, 0, 0, 0};
            can_master.tx_int16(GroupId::PAYLOAD, device_id, message);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
