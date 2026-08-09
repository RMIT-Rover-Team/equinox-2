#include "CommsThread.h"


CommsThread::CommsThread(const char *can_interface) : can_bus(can_interface), can_master(can_bus, 0x0), gimbal(can_master) {}
CommsThread::~CommsThread() {
    stop();
}

bool CommsThread::excavator_alive() {
    return excavator_tilt_alive;
}

bool CommsThread::bucket_alive() {
    return bucket_tilt_alive;
}

void CommsThread::estop() {
    {
        std::lock_guard<std::mutex> lock(m_gimbal);
        gimbal.estop();
    }

    std::lock_guard<std::mutex> lock(m_target);
    target_tilt_speed = 0;
    last_sent_tilt_speed = 0;
    target_pan_position = 0;
    last_sent_pan_position = 0;
}

void CommsThread::set_tilt_speed(int8_t speed) {
    std::lock_guard<std::mutex> lock(m_target);
    target_tilt_speed = speed;
}

void CommsThread::set_pan_position(int8_t position) {
    std::lock_guard<std::mutex> lock(m_target);
    target_pan_position = position;
}

int16_t CommsThread::get_tilt_speed() {
    std::lock_guard<std::mutex> lock(m_target);
    return target_tilt_speed;
}

int16_t CommsThread::get_pan_position() {
    std::lock_guard<std::mutex> lock(m_target);
    return target_pan_position;
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
            std::lock_guard<std::mutex> lock(m_target);
            std::lock_guard<std::mutex> lock2(m_gimbal);

            // update both at once if we can to save bandwidth
            if (target_tilt_speed != last_sent_tilt_speed && target_pan_position != last_sent_pan_position) {
                gimbal.set_tilt_and_pan(target_tilt_speed, target_pan_position);
                last_sent_tilt_speed = target_tilt_speed;
                last_sent_pan_position = target_pan_position;
            } else {
                if (target_tilt_speed != last_sent_tilt_speed) {
                    gimbal.set_tilt_speed(target_tilt_speed);
                    last_sent_tilt_speed = target_tilt_speed;
                }

                if (target_pan_position != last_sent_pan_position) {
                    gimbal.set_pan_position(target_pan_position);
                    last_sent_pan_position = target_pan_position;
                }
            }
        }

        std::unique_lock<std::mutex> lock(m_target);
        cv.wait_for(lock, std::chrono::milliseconds(100));
    }
}