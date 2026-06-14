#include "RoverCanSlave.h"
#include "GenericCan.h"
#include "CommandUtils.h"
#include <stdio.h>

#ifdef __AVR__
#include <string.h>
#else
#include <cstring>
#endif

RoverCanSlave::RoverCanSlave(uint8_t slave_id, GenericCan* can_bus) : can(can_bus) {
    #ifdef DEBUG
        printf("CAN Slave initialized with ID: %02x", m_slave_id);
    #endif

    //Set up everything
    this->m_slave_id = slave_id;
    this->can = can_bus;

}

void RoverCanSlave::listen() {

    //Command Buffer
    Command myCommand;
    Command returnCommand;

    
    //Await a message to me from any device
    //Upper 6 bits are the destination, lower 6 bits are the source
    //This means that the netmask should be 0xfc0 (0b111111000000)
    CANFrame receivedMSG = can->readMSGFrom(this->m_slave_id << 6, 0xfc0, 500);
    
    //This prevents the function from running further and sending the dummy msg
    if (receivedMSG.can_id == 0){
        #ifdef DEBUG 
        Serial.println("Timed out"); 
        #endif
        return; 
    }

    //Extract the command and identifier
    uint8_t command_id, identifier_id;
    
    //Decode the parameters
    myCommand.clearCommand();
    memcpy(myCommand.getBuffer(), receivedMSG.data, receivedMSG.can_dlc);

    this->unpack4(myCommand.getNextUint8(), &command_id, &identifier_id);

    //Get the dest and source so we can reply
    uint8_t destination_id, source_id;
    RoverCanSlave::parseCanID(receivedMSG.can_id, &destination_id, &source_id);

    //Decode the message
    uint8_t MotorOrChannelID, Flags;
    RoverCanSlave::unpack4(myCommand.getNextUint8(), &MotorOrChannelID, &Flags);

    //Debug dump the data
    #ifdef DEBUG 
    printf("Received from %02x to %02x command %02x identifier %02x\n", source_id, destination_id, command_id, identifier_id);
    printf("Motor %02x Flags %02x\n", MotorOrChannelID, Flags);
    #endif

    //Extract the floating value
    float received_data = myCommand.getNextFloat();

    //Start building the return command
    returnCommand.clearCommand();
    //Add the command ID
    returnCommand.addUInt8( RoverCanSlave::pack4(command_id, identifier_id) );

    //Add motor ID and flags
    returnCommand.addUInt8( RoverCanSlave::pack4(MotorOrChannelID, Flags) );

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
            break;
    }

    //Pack the return value
    returnCommand.addFloat(floatResult);

    #ifdef DEBUG
    printf("Returning Float %lf\n", floatResult);
    printf("Replying to %02x from %02x\n", source_id, destination_id);
    #endif

    //Send the return message
    can->writeMSG(RoverCanSlave::generateCanID(destination_id, source_id), (char*)returnCommand.getBuffer(), returnCommand.getBufferLength());

}

void RoverCanSlave::noBlockListenTick() {
    //Command Buffer
    Command myCommand;
    Command returnCommand;

    
    //Await a message to me from any device
    //Upper 6 bits are the destination, lower 6 bits are the source
    //This means that the netmask should be 0xfc0 (0b111111000000)
    if (can->availableFrom(this->m_slave_id << 6, 0xfc0)){
        CANFrame receivedMSG = can->readMSGFrom(this->m_slave_id << 6, 0xfc0);

        //Extract the command and identifier
        uint8_t command_id, identifier_id;
        
        //Decode the parameters
        myCommand.clearCommand();
        memcpy(myCommand.getBuffer(), receivedMSG.data, receivedMSG.can_dlc);

        this->unpack4(myCommand.getNextUint8(), &command_id, &identifier_id);

        //Get the dest and source so we can reply
        uint8_t destination_id, source_id;
        RoverCanSlave::parseCanID(receivedMSG.can_id, &destination_id, &source_id);

        //Decode the message
        uint8_t MotorOrChannelID, Flags;
        RoverCanSlave::unpack4(myCommand.getNextUint8(), &MotorOrChannelID, &Flags);

        //Debug dump the data
        #ifdef DEBUG 
        printf("Received from %02x to %02x command %02x identifier %02x\n", source_id, destination_id, command_id, identifier_id);
        printf("Motor %02x Flags %02x\n", MotorOrChannelID, Flags);
        #endif

        //Extract the floating value
        float received_data = myCommand.getNextFloat();

        //Start building the return command
        returnCommand.clearCommand();
        //Add the command ID
        returnCommand.addUInt8( RoverCanSlave::pack4(command_id, identifier_id) );

        //Add motor ID and flags
        returnCommand.addUInt8( RoverCanSlave::pack4(MotorOrChannelID, Flags) );

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
                break;
        }

        //Pack the return value
        returnCommand.addFloat(floatResult);

        #ifdef DEBUG
        printf("Returning Float %lf\n", floatResult);
        printf("Replying to %02x from %02x\n", source_id, destination_id);
        #endif

        //Send the return message
        can->writeMSG(RoverCanSlave::generateCanID(destination_id, source_id), (char*)returnCommand.getBuffer(), returnCommand.getBufferLength());

    }
}

void RoverCanSlave::broadcastDP(int streamID, int channelID, double value){
    Command myCommand;

    const uint8_t command_id = 0x7;

    uint16_t broadcast_can_id = RoverCanSlave::generateCanID(m_slave_id, 0xFF); // Broadcast to all

    //Add Command and Stream
    myCommand.addUInt8( ((command_id & 0x0f) << 4) | (streamID & 0x0f) );
    // Add channel ID
    myCommand.addUInt8( ((channelID & 0x0f) << 4) | (0 & 0x0f) );

    //Add data point
    myCommand.addFloat(value);

    can->writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());
}



uint16_t RoverCanSlave::generateCanID(uint8_t source, uint8_t dest) {
    uint16_t canID = 0;
    canID |= (dest & 0x3f) << 6;
    canID |= (source & 0x3f);
    return canID;
}

// Helper functions
void RoverCanSlave::parseCanID(uint16_t can_id, uint8_t* dest_id, uint8_t* source_id) {
    *dest_id = (can_id >> 6) & 0x3f;
    *source_id = can_id & 0x3f;
}

inline uint8_t RoverCanSlave::pack4(uint8_t A, uint8_t B) {
    A &= 0x0f;
    B &= 0x0f;
    
    return ((uint8_t)(A << 4) | B);
}

inline void RoverCanSlave::unpack4(uint8_t IN, uint8_t *A, uint8_t *B) {
    //Jonathan being smart and actually checking the pointers
    if (A) *A = (IN >> 4) & 0x0f;
    if (B) *B = IN & 0x0f;
}

//Default handlers
void RoverCanSlave::handleSimpleCall() {
    printf("Simple call\n");
}

void RoverCanSlave::handleDirectCall(int target) {
    printf("Direct call to %02x\n", target);
}

void RoverCanSlave::handleSetFloatType(int target, double in) {
    printf("Set float type of %02x to %lf\n", target, in);
}

double RoverCanSlave::handleGetFloatType(int target) {
    printf("Get float type of %02x\n", target);
    return 0.123;
}

void RoverCanSlave::handleSetter(int target, uint8_t setV) {
    printf("Setter of %02x to %i\n", target, setV); 
}

double RoverCanSlave::handleRequest(int streamID, int channelID) {
    printf("Request data point of %02x Channel %02x\n", streamID, channelID);
    return 0.123;
}