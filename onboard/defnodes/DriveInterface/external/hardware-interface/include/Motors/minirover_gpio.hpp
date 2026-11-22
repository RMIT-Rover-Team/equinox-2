#pragma once

#include "drive_motor_wrapper.hpp"

#include <array>
#include <atomic>
#include <mutex>
#include <thread>

struct gpiod_chip;
struct gpiod_line_request;

// Owns the mini-rover's GPIO lines and produces software PWM for its H-bridges.
class MiniRoverGpio {
  public:
    explicit MiniRoverGpio(const char* chip_path);
    void enable();
    void disable();
    void setMotorSpeed(int motor_index, float speed);
    ~MiniRoverGpio();

  private:
    static constexpr int kMotorCount = 4;       // LF, LR, RF, RR.
    static constexpr int kLinesPerMotor = 2;    // Forward and reverse per wheel.

    gpiod_chip* chip;
    gpiod_line_request* motor_request;
    gpiod_line_request* enable_request;
    // Duty cycles are signed percentages: positive is forward, negative reverse.
    std::array<std::atomic<int>, kMotorCount> duties;
    // Enable and running are shared with the PWM worker thread.
    std::atomic<bool> enabled;
    std::atomic<bool> running;
    std::thread pwm_thread;
    // Serializes libgpiod output writes from control and PWM threads.
    std::mutex gpio_mutex;

    // Immediately drives every H-bridge direction input low.
    void stopAll();
    // Repeats fixed-duration PWM cycles while the backend is enabled.
    void pwmLoop();
    // Drives one H-bridge direction pair for the active part of a PWM cycle.
    void setMotorOutput(int motor_index, int duty);
};

// Adapts one mini-rover wheel to TorqueHandler's existing motor interface.
class MiniRoverGpioMotor : public DriveMotorWrapper {
  public:
    MiniRoverGpioMotor(MiniRoverGpio* gpio, int motor_index, int direction);

    void calibrate() override;
    void enable() override;
    void disable() override;
    void setTorque(float value) override;
    void setSpeed(float value) override;
    void clearErrors() override;
    void estop() override;
    float getSpeed() override;
    float getPos() override;

  private:
    MiniRoverGpio* gpio;
    // Identifies the wheel in MiniRoverGpio's LF, LR, RF, RR pin order.
    int motor_index;
    // Allows a wheel's physical direction to be inverted without rewiring.
    int direction;
    // Stores the requested speed because this open-loop backend has no encoder.
    float speed;
};
