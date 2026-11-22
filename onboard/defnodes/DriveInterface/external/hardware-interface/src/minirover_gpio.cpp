#include "Motors/minirover_gpio.hpp"

#include <gpiod.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <stdexcept>

namespace {
// BCM GPIO offsets equivalent to the original WiringPi pin assignments.
// Each consecutive pair is the forward/reverse input for LF, LR, RF, then RR.
constexpr std::array<unsigned int, 8> kMotorOffsets = {
    13, 12, 19, 16, 18, 17, 15, 14,
};
// BCM GPIO offsets for the five H-bridge enable lines.
constexpr std::array<unsigned int, 5> kEnableOffsets = {
    27, 21, 20, 4, 2,
};

// Requests a group of output lines with a descriptive consumer name for gpioinfo.
gpiod_line_request* requestOutputLines(
    gpiod_chip* chip,
    const unsigned int* offsets,
    std::size_t offset_count,
    const char* consumer
) {
    gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_config* line_config = gpiod_line_config_new();
    gpiod_request_config* request_config = gpiod_request_config_new();

    if (settings == nullptr || line_config == nullptr || request_config == nullptr) {
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_config);
        gpiod_request_config_free(request_config);
        throw std::runtime_error("Unable to allocate libgpiod configuration");
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_request_config_set_consumer(request_config, consumer);

    if (gpiod_line_config_add_line_settings(line_config, offsets, offset_count, settings) < 0) {
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_config);
        gpiod_request_config_free(request_config);
        throw std::runtime_error("Unable to configure GPIO output lines");
    }

    gpiod_line_request* request = gpiod_chip_request_lines(chip, request_config, line_config);
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_config);
    gpiod_request_config_free(request_config);

    if (request == nullptr) {
        throw std::runtime_error("Unable to request GPIO output lines");
    }

    return request;
}

// Controller sticks are normally -1.0 to 1.0; preserve larger percentage inputs.
int speedToDuty(float speed) {
    const float normalized_speed = std::abs(speed) <= 1.0F ? speed * 100.0F : speed;
    return static_cast<int>(std::lround(std::clamp(normalized_speed, -100.0F, 100.0F)));
}
}

MiniRoverGpio::MiniRoverGpio(const char* chip_path)
    : chip(gpiod_chip_open(chip_path)), motor_request(nullptr), enable_request(nullptr), duties{},
      enabled(false), running(true) {
    if (chip == nullptr) {
        throw std::runtime_error("Unable to open GPIO chip");
    }

    try {
        // Keep direction and enable lines as separate requests for independent control.
        motor_request = requestOutputLines(chip, kMotorOffsets.data(), kMotorOffsets.size(), "minirover-drive");
        enable_request = requestOutputLines(chip, kEnableOffsets.data(), kEnableOffsets.size(), "minirover-enable");
    } catch (...) {
        if (motor_request != nullptr) {
            gpiod_line_request_release(motor_request);
        }
        if (enable_request != nullptr) {
            gpiod_line_request_release(enable_request);
        }
        gpiod_chip_close(chip);
        throw;
    }

    // Start safe: all duty cycles remain zero until DriveInterface explicitly enables.
    for (auto& duty : duties) {
        duty.store(0);
    }
    pwm_thread = std::thread(&MiniRoverGpio::pwmLoop, this);
}

void MiniRoverGpio::enable() {
    // Enabling permits the worker to emit PWM before turning on the H-bridges.
    enabled.store(true);
    std::lock_guard<std::mutex> lock(gpio_mutex);
    for (const unsigned int offset : kEnableOffsets) {
        gpiod_line_request_set_value(enable_request, offset, GPIOD_LINE_VALUE_ACTIVE);
    }
}

void MiniRoverGpio::disable() {
    // Stop direction outputs before removing the H-bridge enable signal.
    enabled.store(false);
    stopAll();
    std::lock_guard<std::mutex> lock(gpio_mutex);
    for (const unsigned int offset : kEnableOffsets) {
        gpiod_line_request_set_value(enable_request, offset, GPIOD_LINE_VALUE_INACTIVE);
    }
}

void MiniRoverGpio::setMotorSpeed(int motor_index, float speed) {
    if (motor_index < 0 || motor_index >= kMotorCount) {
        throw std::out_of_range("Invalid mini-rover motor index");
    }

    // The PWM worker reads this atomically at the start of its next cycle.
    duties[motor_index].store(speedToDuty(speed));
}

void MiniRoverGpio::stopAll() {
    // Clearing the requested duties prevents the next PWM cycle from restarting motion.
    for (auto& duty : duties) {
        duty.store(0);
    }
    std::lock_guard<std::mutex> lock(gpio_mutex);
    for (const unsigned int offset : kMotorOffsets) {
        gpiod_line_request_set_value(motor_request, offset, GPIOD_LINE_VALUE_INACTIVE);
    }
}

void MiniRoverGpio::setMotorOutput(int motor_index, int duty) {
    const unsigned int forward_offset = kMotorOffsets[motor_index * kLinesPerMotor];
    const unsigned int reverse_offset = kMotorOffsets[motor_index * kLinesPerMotor + 1];
    gpiod_line_request_set_value(motor_request, forward_offset,
        duty > 0 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(motor_request, reverse_offset,
        duty < 0 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

void MiniRoverGpio::pwmLoop() {
    using Clock = std::chrono::steady_clock;
    // 100 Hz matches the simple, non-real-time nature of this GPIO drive backend.
    constexpr auto period = std::chrono::milliseconds(10);

    while (running.load()) {
        const auto cycle_start = Clock::now();
        std::array<int, kMotorCount> cycle_duties{};

        if (!enabled.load()) {
            // Avoid GPIO writes while disabled but wake regularly for a new enable request.
            std::this_thread::sleep_until(cycle_start + period);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(gpio_mutex);
            if (!enabled.load()) {
                // A disable request won the race before this cycle started.
                continue;
            }
            for (int index = 0; index < kMotorCount; ++index) {
                cycle_duties[index] = duties[index].load();
                setMotorOutput(index, cycle_duties[index]);
            }
        }

        // Each wheel is turned off at its duty percentage within the 10 ms cycle.
        int completed_percent = 0;
        while (completed_percent < 100 && running.load() && enabled.load()) {
            int next_percent = 100;
            // Find the next distinct duty edge to minimize sleep and GPIO operations.
            for (const int duty : cycle_duties) {
                const int duty_percent = std::abs(duty);
                if (duty_percent > completed_percent && duty_percent < next_percent) {
                    next_percent = duty_percent;
                }
            }

            std::this_thread::sleep_until(cycle_start + period * next_percent / 100);
            std::lock_guard<std::mutex> lock(gpio_mutex);
            if (!enabled.load()) {
                // disable() already forced the outputs low.
                break;
            }
            for (int index = 0; index < kMotorCount; ++index) {
                if (std::abs(cycle_duties[index]) == next_percent) {
                    setMotorOutput(index, 0);
                }
            }
            completed_percent = next_percent;
        }
    }
}

MiniRoverGpio::~MiniRoverGpio() {
    // Joining first prevents the worker from using released libgpiod requests.
    running.store(false);
    if (pwm_thread.joinable()) {
        pwm_thread.join();
    }
    if (motor_request != nullptr) {
        stopAll();
        gpiod_line_request_release(motor_request);
    }
    if (enable_request != nullptr) {
        gpiod_line_request_release(enable_request);
    }
    if (chip != nullptr) {
        gpiod_chip_close(chip);
    }
}

MiniRoverGpioMotor::MiniRoverGpioMotor(MiniRoverGpio* gpio, int motor_index, int direction)
    : gpio(gpio), motor_index(motor_index), direction(direction), speed(0.0F) {}

// GPIO H-bridges have no calibration or controller error state.
void MiniRoverGpioMotor::calibrate() {}

void MiniRoverGpioMotor::enable() { gpio->enable(); }

void MiniRoverGpioMotor::disable() { gpio->disable(); }

void MiniRoverGpioMotor::setTorque(float) {}

void MiniRoverGpioMotor::setSpeed(float value) {
    // Keep the caller's orientation while applying any wheel-specific inversion at the GPIO edge.
    speed = value;
    gpio->setMotorSpeed(motor_index, value * direction);
}

void MiniRoverGpioMotor::clearErrors() {}

void MiniRoverGpioMotor::estop() { gpio->disable(); }

// Return the command, not a measured velocity; the mini rover has no encoder feedback.
float MiniRoverGpioMotor::getSpeed() { return speed; }

// Position feedback is unavailable for this open-loop motor driver.
float MiniRoverGpioMotor::getPos() { return 0.0F; }
