#pragma once
#include "CommandUtils.h"

typedef enum {
    CMD_ESTOP               = 0x0,
    CMD_CALIBRATE_RESET     = 0x1,
    CMD_SET_MOTOR_POSITION  = 0x2,
    CMD_SET_MOTOR_SPEED     = 0x3,
    CMD_TOGGLE_STATE        = 0x4,
    CMD_GET_MOTOR_POSITION  = 0x5,
    CMD_GET_MOTOR_SPEED     = 0x6,
    CMD_BROADCAST_DATAPOINT = 0x7,
    CMD_REQUEST_DATAPOINT   = 0x8,
    CMD_PING                = 0x9
} CommandID;


class RoverMiniSlave {
public:
    RoverMiniSlave(uint8_t slave_id);

    // Injest a buffer (expected 8 bytes long), 
    bool injest(uint32_t* canID, unsigned char* canBuffer);
    void broadcastDP(uint32_t* canID, unsigned char* canBuffer, int streamID, int channelID, double value);

    // Function pointers for command handlers   
    void (*handleEStop)() = &RoverMiniSlave::handleSimpleCall;
    void (*handleCalibrate)(int motor_id) = &RoverMiniSlave::handleDirectCall;
    void (*handleSetMotorPosition)(int motor_id, double position) = &RoverMiniSlave::handleSetFloatType;
    void (*handleSetMotorSpeed)(int motor_id, double speed) = &RoverMiniSlave::handleSetFloatType;
    double (*handleGetMotorPosition)(int motor_id) = &RoverMiniSlave::handleGetFloatType;
    double (*handleGetMotorSpeed)(int motor_id) = &RoverMiniSlave::handleGetFloatType;
    void (*handleToggleState)(int motor_id, uint8_t state) = &RoverMiniSlave::handleSetter;
    double (*handleRequestDataPoint)(int streamID, int channelID) = &RoverMiniSlave::handleRequest;

private:    
    uint8_t m_slave_id;

    static uint16_t generateCanID(uint8_t source, uint8_t dest);
    static void parseCanID(uint16_t can_id, uint8_t* dest_id, uint8_t* source_id);

    //Dummy handlers for uninitialised commands
    static void handleSimpleCall();
    static void handleDirectCall(int target);
    static void handleSetFloatType(int target, double in);
    static double handleGetFloatType(int target);
    static void handleSetter(int target, uint8_t setV);
    static double handleRequest(int streamID, int channelID);


    inline static uint8_t pack4(uint8_t A, uint8_t B);
    inline static void unpack4(uint8_t IN, uint8_t *A, uint8_t *B);
    
};