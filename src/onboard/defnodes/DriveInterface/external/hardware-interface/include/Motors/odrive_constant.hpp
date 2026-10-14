// TODO: update this file

#pragma once 
#include <cstdint>
#include <map>

enum ODriveCommand : uint16_t
{
    Get_Version = 0x000,
    Heartbeat = 0x001,
    Estop = 0x002,
    Get_Error = 0x003,
    RxSdo = 0x004,
    TxSdo = 0x005,
    Address = 0x006,
    Set_Axis_State = 0x007,
    Get_Encoder_Estimates = 0x009,
    Set_Controller_Mode = 0x00b,
    Set_Input_Pos = 0x00c,
    Set_Input_Vel = 0x00d,
    Set_Input_Torque = 0x00e,
    Set_Limits = 0x00f,
    Set_Traj_Vel_Limit = 0x011,
    Set_Traj_Accel_Limits = 0x012,
    Set_Traj_Inertia = 0x013,
    Get_Iq = 0x014,
    Get_Temperature = 0x015,
    Reboot = 0x016,
    Get_Bus_Voltage_Current = 0x017,
    Clear_Errors = 0x018,
    Set_Absolute_Position = 0x019,
    Set_Pos_Gain = 0x01a,
    Set_Vel_Gains = 0x01b,
    Get_Torques = 0x01c,
    Get_Powers = 0x01d,
    Enter_DFU_Mode = 0x01f
};

enum ODriveAxisState : uint8_t
{
    UNDEFINED = 0x0,
    IDLE = 0x1, // Idle (For after calibrate)
    STARTUP_SEQUENCE = 0x2,
    FULL_CALIBRATION_SEQUENCE = 0x3, // Calibration sequence
    MOTOR_CALIBRATION = 0x4,
    ENCODER_INDEX_SEARCH = 0x6,
    ENCODER_OFFSET_CALIBRATION = 0x7,
    CLOSED_LOOP_CONTROL = 0x8, // Normal 'motor on' mode
    LOCKIN_SPIN = 0x9,
    ENCODER_DIR_FIND = 0x10,
    HOMING = 0x11,
    ENCODER_HALL_POLARITY_CALIBRATION = 0xc,
    ENCODER_HALL_PHASE_CALIBRATION = 0xd,
    ANTICOGGING_CALIBRATION = 0xe
};

enum ODriveControlMode : uint16_t
{
    VOLTAGE_CONTROL = 0x0,
    TORQUE_CONTROL = 0x1,
    VELOCITY_CONTROL = 0x2,
    POSITION_CONTROL = 0x3
};

enum ODriveInputMode : uint16_t
{
    INACTIVE = 0x0,
    PASSTHROUGH = 0x1,
    VEL_RAMP = 0x2,
    POS_FILTER = 0x3,
    TRAP_TRAJ = 0x5,
    TORQUE_RAMP = 0x6,
    MIRROR = 0x7,
    TUNING = 0x8
};