// TODO: update this file

#include "odrive_control.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <net/if.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdio.h>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>

ODriveController::ODriveController(const char *interface)
{
    CANSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (!CANSocket)
    {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct ifreq ifr;
    std::strcpy(ifr.ifr_name, interface);

    const int ifaceResult = ioctl(CANSocket, SIOCGIFINDEX, &ifr);
    if (ifaceResult == -1)
    {
        perror(nullptr);
        std::cerr << "Error assigning interface " + std::string(interface) + "\n";
        return;
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    const int bindResult = bind(CANSocket, (struct sockaddr *)&addr, sizeof(addr));
    if (bindResult == -1)
    {
        perror(nullptr);
        std::cerr << "Error binding\n";
        return;
    }

    // TODO
    
    // CAN is ready
    socketReady = true;

    // Start listening thread
    if (socketReady)
    {
        listener_thread = std::thread(&ODriveController::listenCAN, this);
    }

    
}

ODriveController::~ODriveController()
{
    // Close CANSocket if was opened.
    if (CANSocket)
        close(CANSocket);
    
    listening = 0;
    if (listener_thread.joinable())
        listener_thread.join();

}

ODriveState ODriveController::getState(const uint8_t& id) const
{
    std::lock_guard<std::mutex> lock(m_odrive_states);
    return odrive_states.at(id);
}

void ODriveController::listenCAN()
{
    struct can_frame frame;
    while (listening)
    {
        // Wait for a frame
        const int readResult = read(CANSocket, &frame, sizeof(frame));
        if (readResult <= 0)
        {
            perror(nullptr);
            std::cerr << "Error reading from CAN socket\n";
            continue;
        }

        // Check if the frame is valid
        if (readResult < sizeof(struct can_frame))
        {
            std::cerr << "Received invalid CAN frame\n";
            continue;
        }

        // Extract ID and command from the frame
        uint8_t id = (frame.can_id >> 5); // Extracting ID from CAN ID
        uint16_t cmd_id = frame.can_id & 0x1F; // Extracting command

        if (cmd_id == ODriveCommand::Get_Encoder_Estimates)
        {
            float pos = 0.0f;
            float vel = 0.0f;

            std::memcpy(&pos, frame.data, 4); // First 4 bytes are position
            std::memcpy(&vel, frame.data + 4, 4); // Last 4 bytes are velocity

            std::lock_guard<std::mutex> lock(m_odrive_states);
            odrive_states[id].Pos_Estimate = pos;
            odrive_states[id].Vel_Estimate = vel;
            odrive_states[id].last_updated = std::chrono::system_clock::now();
        }
        else if (cmd_id == ODriveCommand::Heartbeat)
        {
            //std::cerr << "Heartbeat!\n";
        }
        else
        {
            //std::cerr << "Unknown command ID: " << cmd_id << "\n";
        }
    }
}

bool ODriveController::requestEncoder(const uint8_t& id)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to request encoder with invalid CAN socket!\n";
        return false;
    }

    return sendCommand(id, ODriveCommand::Get_Encoder_Estimates, {0});
}

bool ODriveController::sendCommand(const uint8_t &id, const ODriveCommand &cmd, const std::array<uint8_t, 8> &data) const
{
    struct can_frame frame;

    // CANSimple addressing
    frame.can_id = (id << 5) | cmd;

    // Send 8 byte CAN frame
    frame.len = 8;
    std::memcpy(frame.data, data.data(), data.size());

    const int writeResult = write(CANSocket, &frame, sizeof(frame));
    if (writeResult == -1)
    {
        perror(nullptr);
        std::cerr << "Failed to send frame to CAN\n";
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            std::cerr << "can bus send timeout\n";
        }
        return false;
    }

    // Assume success
    return true;
}

bool ODriveController::CANConnected() const
{
    return socketReady;
}

bool ODriveController::setAxisState(const uint8_t &id, const ODriveAxisState &state) const
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setVelocity with invalid CAN socket!\n";
        return false;
    }

    // setAxisState takes 32bit int of the state.

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &state, 8); // First 4 bytes are velocity

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Axis_State, payload);
}

bool ODriveController::clearErrors(const uint8_t &id)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setVelocity with invalid CAN socket!\n";
        return false;
    }

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Clear_Errors, {0});
}

bool ODriveController::setVelocity(const uint8_t &id, const float &target_velocity, const float &torque)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setVelocity with invalid CAN socket!\n";
        return false;
    }
    std::cout << "ID: " << (int)id << "Velocity: " << target_velocity << std::endl;
    // setVelocity command takes:
    // Bytes    Name    Type    Unit
    // 0-3      Vel     float   rev/s
    // 4-7      Trq     float   Nm      (Can be left as 0 - allan)

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &target_velocity, 4); // First 4 bytes are velocity
    std::memcpy(payload.data() + 4, &torque, 4);      // Last 4 bytes are torque

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Input_Vel, payload);
}

bool ODriveController::setPosition(const uint8_t &id, const float &target_position, const uint16_t &velocity_ff, const uint16_t torque_ff)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setVelocity with invalid CAN socket!\n";
        return false;
    }

    // Set_Inpit_Pos command takes:
    // Start Byte   Name    Type
    // 0            Pos     float32
    // 4            VelFF   int16
    // 6            TrqFF   int16

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &target_position, 4); // First 4 bytes are position
    std::memcpy(payload.data() + 4, &velocity_ff, 2); // Next 2 bytes are vel
    std::memcpy(payload.data() + 6, &torque_ff, 2);   // Next 2 bytes are vel

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Input_Pos, payload);
}

bool ODriveController::setTorque(const uint8_t &id, const float &target_torque)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setTorque with invalid CAN socket!\n";
        return false;
    }

    // setTorque command takes:
    // Bytes    Name    Type    Unit
    // 0-3      Trq     float   Nm

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &target_torque, 4); // First 4 bytes are torque

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Input_Torque, payload);
}

bool ODriveController::setAccelLim(const uint8_t& id, const float& accel_limit){
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setAccel with invalid CAN socket!\n";
        return false;
    }

    // setTorque command takes:
    // Bytes    Name    Type    Unit
    // 0-3      Accel     float   
    // Bytes    Name    Type    Unit
    // 4-7      Deccel     float   

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &accel_limit, 4); // First 4 bytes are Accel
    std::memcpy(payload.data()+4, &accel_limit, 4); // First 4 bytes are Deccel

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Traj_Accel_Limits, payload);
}

bool ODriveController::setControlMode(const uint8_t &id, const ODriveControlMode &control_mode, const ODriveInputMode &input_mode)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to setVelocity with invalid CAN socket!\n";
        return false;
    }

    // setVelocity command takes:
    // Bytes    Name    Type    Unit
    // 0-3      Vel     float   rev/s
    // 4-7      Trq     float   Nm      (Can be left as 0 - allan)

    std::array<uint8_t, 8> payload = {0};
    std::memcpy(payload.data(), &control_mode, 4);   // First 4 bytes are velocity
    std::memcpy(payload.data() + 4, &input_mode, 4); // Last 4 bytes are torque

    // Return whether or not packet was sent
    return sendCommand(id, ODriveCommand::Set_Controller_Mode, payload);
}

bool ODriveController::estop(const uint8_t& id)
{
    if (!CANConnected())
    {
        std::cerr << "[ODriveController] Error: Tried to send estop with invalid CAN socket!\n";
        return false;
    }

    std::array<uint8_t, 8> payload = {0};  // estop has an empty 8-byte payload
    return sendCommand(id, ODriveCommand::Estop, payload);
}
