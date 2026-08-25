// to do
// change naming of some variables
// make test cases
// fill out all identifier ids
// add validation?
// fix reads to actually work properly instead of just reading every output.

#include "RoverCanMaster.h"
#include "GenericCan.h"
#include "CommandUtils.h"
#include <iostream>
#include <utility>
#include <tuple>
#include <cstring>
#include <stdint.h>


RoverCanMaster::RoverCanMaster(GenericCan& myCan, uint8_t myID)
    : can(myCan), myID(myID), identifierCounter(0) {
}


uint16_t RoverCanMaster::generateCanID(uint8_t source, uint8_t dest){
    uint16_t canID = 0;
    canID |= (dest & 0x3f) << 6;
    canID |= (source & 0x3f);
    return canID;
}


bool RoverCanMaster::EStop(uint8_t destID) {
    //printf("Estop\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x0;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte

    return myCommand.getNextUint8(); // State Byte
}   


bool RoverCanMaster::Calibrate(uint8_t destID, int motor_id) {
    //printf("Calibrate\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x1;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (0 & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte

    return myCommand.getNextUint8(); // State Byte
}

ReceivedState RoverCanMaster::SetMotorPosition(uint8_t destID, int motor_id, float position) {
    //printf("Set motor position\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x2;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (0 & 0xf) );
    myCommand.addFloat(position);
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte
    myCommand.getNextUint8(); // Motor ID Byte

    ReceivedState received_state = convertReceivedState(receivedMSG);
    return received_state;
}

ReceivedState RoverCanMaster::SetMotorSpeed(uint8_t destID, int motor_id, float speed) {
    //printf("Set motor speed\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x3;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (0 & 0xf) );
    myCommand.addFloat(speed);
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte
    myCommand.getNextUint8(); // Motor ID Byte

    ReceivedState received_state = convertReceivedState(receivedMSG);
    return received_state;
}

ReceivedState RoverCanMaster::ToggleState(uint8_t destID, int motor_id, bool toggle_state) {
    //printf("Toggle motor state\n");
    Command myCommand;
    
    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x4;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (toggle_state & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte
    myCommand.getNextUint8(); // Motor ID Byte

    ReceivedState received_state = convertReceivedState(receivedMSG);
    return received_state;
}

std::pair<ReceivedState, float> RoverCanMaster::GetMotorPosition(uint8_t destID, int motor_id) {
    //printf("Get motor position\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x5;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (0 & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffffffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte
    myCommand.getNextUint8(); // Motor ID Byte

    float received_position = myCommand.getNextFloat();
    ReceivedState received_state = convertReceivedState(receivedMSG);

    return {received_state, received_position};
}

std::pair<ReceivedState, float> RoverCanMaster::GetMotorSpeed(uint8_t destID, int motor_id) {
    //printf("Get motor speed\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x6;
    uint8_t identifier_id = this->identifierCounter++;

    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    myCommand.addUInt8( ((motor_id & 0xf) << 4) | (0 & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    // READ MSG
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffff); // Netmask all 1s to make sure we get any
    memcpy(myCommand.getBuffer(),receivedMSG.data,8);

    myCommand.getNextUint8(); // Command Byte
    myCommand.getNextUint8(); // Motor ID Byte

    float received_speed = myCommand.getNextFloat();
    ReceivedState received_state = convertReceivedState(receivedMSG);

    return {received_state, received_speed};
}

std::pair<Datapoint, float> RoverCanMaster::BroadcastDataPoint() {
    //printf("Broadcast data point\n");
    Command myCommand;

    //Mask for upper 6 bits matching all 1s
    CANFrame receivedMSG = can.readMSGFrom(0x0fc0, 0x0fc0);

    Datapoint datapoint;
    datapoint.stream_id = (receivedMSG.data[0] & 0x0f) << 4;
    datapoint.channel_id = receivedMSG.data[1] & ~0x0f;
    float received_data;
    memcpy(&received_data, &receivedMSG.data[2], sizeof(float));

    return {datapoint, received_data};
}

std::pair<Datapoint, float> RoverCanMaster::RequestDataPoint(uint8_t destID,int stream_id, int channel_id) {
    //printf("Request data point\n");
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);
    char data[8] = {0};

    uint8_t command_id = 0x8;
    
    data[0] = (command_id << 4) | stream_id;
    data[1] = (channel_id & 0x0f) << 4;

    can.writeMSG(broadcast_can_id, data, sizeof(data));

    CANFrame receivedMSG = can.readMSG();

    Datapoint datapoint;
    datapoint.stream_id = (receivedMSG.data[0] & 0x0f) << 4;
    datapoint.channel_id = receivedMSG.data[1] & ~0x0f;
    float received_data;
    memcpy(&received_data, &receivedMSG.data[2], sizeof(float));

    return {datapoint, received_data};
}



ReceivedState RoverCanMaster::convertReceivedState(CANFrame &receivedMSG) {
    ReceivedState received_state;
    received_state.motor_id = receivedMSG.data[1] & ~0x0f;
    received_state.error_flag = (receivedMSG.data[1] >> 3) & 0x01;
    received_state.uncallibrated_flag = (receivedMSG.data[1] >> 2) & 0x01;
    return received_state;
}


bool RoverCanMaster::ping(uint8_t destID) {
    //printf("Ping\n");
    Command myCommand;

    // WRITE MSG
    uint16_t broadcast_can_id = generateCanID(this->myID, destID);

    uint8_t command_id = 0x9;
    uint8_t identifier_id = this->identifierCounter++;
    myCommand.addUInt8( ((command_id & 0xf) << 4) | (identifier_id & 0xf) );
    can.writeMSG(broadcast_can_id, (char*)myCommand.getBuffer(), myCommand.getBufferLength());

    //Await Reply
    myCommand.clearCommand();
    uint16_t expected_response_can_id = generateCanID(destID, this->myID);

    CANFrame receivedMSG = can.readMSGFrom(expected_response_can_id, 0xffff, 1000); // Netmask all 1s to make sure we get any
    if (receivedMSG.can_id == 0){
        return false;
    }
    return true;

}


    