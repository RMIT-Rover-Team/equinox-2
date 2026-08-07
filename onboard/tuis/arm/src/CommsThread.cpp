#include "CommsThread.h"


CommsThread::CommsThread(GenericCan &can_bus) : payload(can_bus) {}
CommsThread::~CommsThread() {
    stop();
}

void CommsThread::estop() {
    std::lock_guard<std::mutex> lock(m_payload);
    payload.estop();
}

void CommsThread::start() {
    worker = std::thread(&CommsThread::run, this);
}

/// @brief Kills the worker thread. Note that it is blocking until the worker next reads the command queue
void CommsThread::stop() {
    std::lock_guard<std::mutex> lock(m_should_stop);
    should_stop = true;
    if (worker.joinable()) worker.join();
}

/// @brief  Ran on the worker thread
void CommsThread::run() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_should_stop);
            if (should_stop) break;
        }

        {
            std::lock_guard<std::mutex> lock(m_target_state);
            std::lock_guard<std::mutex> lock2(m_payload);

            // MyActuators
            for (int i = 0; i < 6; ++i) {
                if (target_state.motor_positions.at(i) != last_sent_state.motor_positions.at(i)) {
                    payload.motors.at(i).setPosition(target_state.motor_positions.at(i));
                    last_sent_state.motor_positions.at(i) = target_state.motor_positions.at(i);
                }
            }

            // end effector grip
            if (target_state.grip_velocity != last_sent_state.grip_velocity) {
                payload.end_effector.set_grip_velocity(target_state.grip_velocity);
                last_sent_state.grip_velocity = target_state.grip_velocity;
            }

            // end effector poke
            if (target_state.poke_velocity != last_sent_state.poke_velocity) {
                payload.end_effector.set_poke_velocity(target_state.poke_velocity);
                last_sent_state.poke_velocity = target_state.poke_velocity;
            }

            // get motor positions
            // note that get_position() is blocking, should probably make it non-blocking
            for (int i = 0; i < 6; ++i) {
                std::cout << "getpos " << i << std::endl;
                encoded_motor_positions.at(i) = payload.motors.at(i).getPosition();
            }
        }

        std::unique_lock<std::mutex> lock(m_target_state);
        cv.wait_for(lock, std::chrono::milliseconds(100));
    }
}