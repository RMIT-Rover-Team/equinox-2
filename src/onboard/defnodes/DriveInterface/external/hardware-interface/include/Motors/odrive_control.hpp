// TODO: update this file

#pragma once
#include <string> 
#include <cstdint>
#include <array>
#include "odrive_constant.hpp"
#include <atomic>
#include <thread>
#include <map>
#include <mutex>
#include <vector>
#include <chrono>

/* 

ODriveController
RMIT Rover

Authors:
* [Command] George
* [Command] Jonathan
* [Command] Allan  

*/

struct ODriveState 
{
    ODriveState() = default;

    uint32_t Axis_Error = 0;
    uint8_t Axis_State = 0;
    uint8_t Procedure_Result = 0;
    uint8_t Trajectory_Done_Flag = 0;

    uint32_t Active_Error = 0;
    uint32_t Disarm_Reason = 0;

    float Pos_Estimate = 0.0f;
    float Vel_Estimate = 0.0f;

    float Iq_Setpoint = 0;
    float Iq_Measured = 0;

    float FET_Temp = 0;
    float Motor_Temp = 0;

    float Bus_Voltage = 0;
    float Bus_Current = 0;

    float Torque_Target = 0;
    float Torque_Estimate = 0;

    std::chrono::system_clock::time_point last_updated;
};

class ODriveController
{
public:
    ODriveController(const char* interface);
    ~ODriveController();

    bool CANConnected() const;

    // void calibrate(const uint8_t& id);
    // void estop(const uint8_t& id);
    // void reboot(const uint8_t& id);

    bool estop(const uint8_t& id);
    
    bool clearErrors(const uint8_t& id);

    bool setAxisState(const uint8_t& id, const ODriveAxisState& state) const;
    bool setControlMode(const uint8_t &id, const ODriveControlMode &control_mode,const ODriveInputMode &input_mode);

    bool setVelocity(const uint8_t& id, const float& target_velocity,const float &torque);
    bool setPosition(const uint8_t& id, const float& target_position, const uint16_t& velocity_ff, const uint16_t torque_ff);
    bool setTorque (const uint8_t& id, const float& target_torque);

    bool setAccelLim(const uint8_t& id, const float& accel_limit);

    bool requestEncoder(const uint8_t& id);
    ODriveState getState(const uint8_t& id) const;
private:

    int CANSocket;
    bool socketReady = false;
    
    bool sendCommand(const uint8_t& id, const ODriveCommand& cmd, const std::array<uint8_t,8>& data) const;

    int messages = 0;

    // Listening: Have a thread to update an atomic list of ODriveStates
    std::map<uint8_t, ODriveState> odrive_states;
    mutable std::mutex m_odrive_states; // Mutex to protect access to odrive_states

    volatile int listening = 1;
    std::thread listener_thread;

    void listenCAN();
};