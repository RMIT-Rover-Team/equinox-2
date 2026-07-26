#include "CommsThread.h"


CommsThread::CommsThread() {}
CommsThread::~CommsThread() {
    stop();
}

void CommsThread::estop() {
    {
        std::lock_guard<std::mutex> lock(m_payload);
        payload.heater.setTargetTemperature(0);
        payload.drill.stop();
    }

    std::lock_guard<std::mutex> lock(m_target_state);
    target_state.heater_temperature = 0;
    target_state.drill_enabled = false;
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

            if (target_state.heater_temperature != last_sent_state.heater_temperature) {
                payload.heater.setTargetTemperature(target_state.heater_temperature);
                last_sent_state.heater_temperature = target_state.heater_temperature;
            }

            if (target_state.drill_height != last_sent_state.drill_height) {
                payload.drill.setHeight(target_state.drill_height);
                last_sent_state.drill_height = target_state.drill_height;
            }

            if (target_state.drill_enabled != last_sent_state.drill_enabled) {
                if (target_state.drill_enabled) payload.drill.start();
                else payload.drill.stop();
                last_sent_state.drill_enabled = target_state.drill_enabled;
            }

            if (target_state.microscope_height != last_sent_state.microscope_height) {
                payload.microscope.setHeight(target_state.microscope_height);
                last_sent_state.microscope_height = target_state.microscope_height;
            }

            if (target_state.microscope_swivel != last_sent_state.microscope_swivel) {
                payload.microscope.setSwivel(target_state.microscope_swivel);
                last_sent_state.microscope_swivel = target_state.microscope_swivel;
            }
        }

        // TODO: change to conditional variable
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
}