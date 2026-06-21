#include "RoverMiniSlave.h"

#include "CommandUtils.h"
#include <stdio.h>

#ifdef __AVR__
#include <string.h>
#else
#include <cstring>
#endif

RoverMiniSlave::RoverMiniSlave(uint8_t slave_id) {
    #ifdef DEBUG
        printf("CAN Slave initialized with ID: %02x", m_slave_id);
    #endif

    //Set up everything
    this->m_slave_id = slave_id;

}

bool RoverMiniSlave::injest(uint32_t* canID, unsigned char* canBuffer) {

    //Command Buffer
    Command myCommand;
    Command returnCommand;

    
    //Await a message to me from any device
    //Upper 6 bits are the destination, lower 6 bits are the source
    
    //Check if it was for us, otherwise skip
    if ((*canID & 0xfc0) != ((uint16_t)m_slave_id << 6)){
        return false;
    }

    //Extract the command and identifier
    uint8_t command_id, identifier_id;
    
    //Decode the parameters
    myCommand.clearCommand();
    memcpy(myCommand.getBuffer(), canBuffer, 8);

    this->unpack4(myCommand.getNextUint8(), &command_id, &identifier_id);

    //Get the dest and source so we can reply
    uint8_t destination_id, source_id;
    RoverMiniSlave::parseCanID(*canID, &destination_id, &source_id);

    //Decode the message
    uint8_t MotorOrChannelID, Flags;
    RoverMiniSlave::unpack4(myCommand.getNextUint8(), &MotorOrChannelID, &Flags);

    //Extract the floating value
    float received_data = myCommand.getNextFloat();

    //Start building the return command
    returnCommand.clearCommand();
    //Add the command ID
    returnCommand.addUInt8( RoverMiniSlave::pack4(command_id, identifier_id) );

    //Add motor ID and flags
    returnCommand.addUInt8( RoverMiniSlave::pack4(MotorOrChannelID, Flags) );

    float floatResult = 0;

    //Switch on the command
    switch (command_id) {
        case CMD_ESTOP:
            //printf("E-Stop requested\n");
            (*handleEStop)();
            break;
        case CMD_CALIBRATE_RESET:
            //printf("Calibrate/reset requested\n");
            (*handleCalibrate)(MotorOrChannelID);
            break;
        case CMD_SET_MOTOR_POSITION:
            //printf("Set motor position requested\n");
            (*handleSetMotorPosition)(MotorOrChannelID, received_data);
            break;
        case CMD_SET_MOTOR_SPEED:
            //printf("Set motor speed requested\n");
            (*handleSetMotorSpeed)(MotorOrChannelID, received_data);
            break;
        case CMD_TOGGLE_STATE:
            //printf("Toggle state requested\n");
            (*handleToggleState)(MotorOrChannelID, Flags);
            break;
        case CMD_GET_MOTOR_POSITION:
            //printf("Get motor position requested\n");
            floatResult = (*handleGetMotorPosition)(MotorOrChannelID);
            break;
        case CMD_GET_MOTOR_SPEED:
            //printf("Get motor speed requested\n");
            floatResult = (*handleGetMotorPosition)(MotorOrChannelID);
            break;

        case CMD_REQUEST_DATAPOINT:
            //printf("Request data point requested\n");
            floatResult = (*handleRequestDataPoint)(identifier_id, MotorOrChannelID);
            break;

        case CMD_PING:
            //Pong!
            memcpy(&floatResult, "PONG", 4);
            break;

        default:
            //printf("Warning: received unknown command %02x\n", command_id);
            return false; // Break out as we don't do anything
            break;
    }

    //Pack the return value
    returnCommand.addFloat(floatResult);

    //Send the return message
    memcpy(canBuffer, returnCommand.getBuffer(), 8);
    *canID = RoverMiniSlave::generateCanID(destination_id, source_id);

    return true;
}

void RoverMiniSlave::broadcastDP(uint32_t* canID, unsigned char* canBuffer, int streamID, int channelID, double value){
    Command myCommand;

    const uint8_t command_id = 0x7;

    uint16_t broadcast_can_id = RoverMiniSlave::generateCanID(m_slave_id, 0xFF); // Broadcast to all

    //Add Command and Stream
    myCommand.addUInt8( ((command_id & 0x0f) << 4) | (streamID & 0x0f) );
    // Add channel ID
    myCommand.addUInt8( ((channelID & 0x0f) << 4) | (0 & 0x0f) );

    //Add data point
    myCommand.addFloat(value);

    //Return
    memcpy(canBuffer, myCommand.getBuffer(), 8);
    *canID = broadcast_can_id;
}



uint16_t RoverMiniSlave::generateCanID(uint8_t source, uint8_t dest) {
    uint16_t canID = 0;
    canID |= (dest & 0x3f) << 6;
    canID |= (source & 0x3f);
    return canID;
}

// Helper functions
void RoverMiniSlave::parseCanID(uint16_t can_id, uint8_t* dest_id, uint8_t* source_id) {
    *dest_id = (can_id >> 6) & 0x3f;
    *source_id = can_id & 0x3f;
}

inline uint8_t RoverMiniSlave::pack4(uint8_t A, uint8_t B) {
    A &= 0x0f;
    B &= 0x0f;
    
    return ((uint8_t)(A << 4) | B);
}

inline void RoverMiniSlave::unpack4(uint8_t IN, uint8_t *A, uint8_t *B) {
    //Jonathan being smart and actually checking the pointers
    if (A) *A = (IN >> 4) & 0x0f;
    if (B) *B = IN & 0x0f;
}

//Default handlers
void RoverMiniSlave::handleSimpleCall() {
    printf("Simple call\n");
}

void RoverMiniSlave::handleDirectCall(int target) {
    printf("Direct call to %02x\n", target);
}

void RoverMiniSlave::handleSetFloatType(int target, double in) {
    printf("Set float type of %02x to %lf\n", target, in);
}

double RoverMiniSlave::handleGetFloatType(int target) {
    printf("Get float type of %02x\n", target);
    return 0.123;
}

void RoverMiniSlave::handleSetter(int target, uint8_t setV) {
    printf("Setter of %02x to %i\n", target, setV); 
}

double RoverMiniSlave::handleRequest(int streamID, int channelID) {
    printf("Request data point of %02x Channel %02x\n", streamID, channelID);
    return 0.123;
}