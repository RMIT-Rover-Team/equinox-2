/*
 * StepperMotor.cpp
 * Implements a CAN-controlled stepper motor interface.
 * set_steps() stores the most recent step command and sends it over CAN.
 * stop() clears the most recent command locally.
 */
 
#include "StepperMotor.h"

StepperMotor::StepperMotor(uint8_t device_id, RoverCanMaster &can_master)
    : device_id(device_id)
    , can_master(can_master)
    , current_steps(0)
    , target_steps(0)
    , target_changed(false)
    , stop_can_worker(false)
{
    can_worker_thread = std::thread(&StepperMotor::can_monitor_loop, this);
}

StepperMotor::~StepperMotor() {
    stop_can_worker = true;
    if (can_worker_thread.joinable())
        can_worker_thread.join();
}

void StepperMotor::set_steps(int16_t steps) {
    std::lock_guard<std::mutex> lock(motor_mutex);
    target_steps = steps;
    target_changed = true;
}

int StepperMotor::get_steps() const {
    return current_steps;
}

void StepperMotor::stop() {
    target_steps = 0;
}

void StepperMotor::can_monitor_loop() {
    while (!stop_can_worker) {
        int16_t local_target_steps = 0;
        bool should_update = false;

        {
            std::lock_guard<std::mutex> lock(motor_mutex);
            if (target_changed) {
                local_target_steps = target_steps;
                should_update = true;
                target_changed = false;
            }
        }

        if (should_update) {
            int16_t message[4] = {local_target_steps, 0, 0, 0};
            can_master.tx_int16(GroupId::PAYLOAD, device_id, message);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
