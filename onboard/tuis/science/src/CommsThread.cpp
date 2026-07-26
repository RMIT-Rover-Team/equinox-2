#include "CommsThread.h"


CommsThread::CommsThread() : can_bus("can2"), can_master(can_bus, 0x0), excavator(can_master) {}
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
        std::lock_guard<std::mutex> lock(m_excavator);
        excavator.estop();
    }

    std::lock_guard<std::mutex> lock(m_target_velocity);
    excavator_target_velocity = 0;
    last_sent_excavator_velocity = 0;
    bucket_target_velocity = 0;
    last_sent_bucket_velocity = 0;
}

void CommsThread::set_excavator_velocity(int16_t velocity) {
    std::lock_guard<std::mutex> lock(m_target_velocity);
    excavator_target_velocity = velocity;
}

void CommsThread::set_bucket_velocity(int16_t velocity) {
    std::lock_guard<std::mutex> lock(m_target_velocity);
    bucket_target_velocity = velocity;
}

int16_t CommsThread::get_excavator_velocity() {
    std::lock_guard<std::mutex> lock(m_target_velocity);
    return excavator_target_velocity;
}

int16_t CommsThread::get_bucket_velocity() {
    std::lock_guard<std::mutex> lock(m_target_velocity);
    return bucket_target_velocity;
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
            std::lock_guard<std::mutex> lock(m_target_velocity);
            std::lock_guard<std::mutex> lock2(m_excavator);

            if (excavator_target_velocity != last_sent_excavator_velocity) {
                excavator.excavator_tilt.set_velocity(excavator_target_velocity);
                last_sent_excavator_velocity = excavator_target_velocity;
            }

            if (bucket_target_velocity != last_sent_bucket_velocity) {
                excavator.bucket_tilt.set_velocity(bucket_target_velocity);
                last_sent_bucket_velocity = bucket_target_velocity;
            }
        }

        // TODO: change to conditional variable
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
}