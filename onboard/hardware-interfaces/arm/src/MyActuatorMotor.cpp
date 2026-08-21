#include "GenericMotor.h"
#include "MyActuatorMotor.h"
#include "GenericCan.h"
#include <stdint.h>
#include <cstring>
#include <iostream>

void __dbg_dump(char* input, int length) {
    //Hexdump the input
    //for (int i = 0; i < length; i++) {
    //    printf("%02X ", (uint8_t)input[i]);
    //}
    //printf("\n");
}

MyActuatorMotor::MyActuatorMotor(int targetID, GenericCan* can) {
  //ctor
  this->myID = targetID;
  this->myCan = can;


  //Init the motor
  this->calibrate();

}


void MyActuatorMotor::calibrate() {
    //Calibrate the motor//
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding[7];
    } myMsg = {0};

    //Clear Errors
    myMsg.command = 0x9b;
    __dbg_dump((char*)&myMsg, sizeof(myMsg));
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));


    //Start Motor
    myMsg.command = 0x88;
    __dbg_dump((char*)&myMsg, sizeof(myMsg));
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));
}



double MyActuatorMotor::getPosition() {
    //Get the position of the motor

    //Clear Historic Values
    while (myCan->availableFrom(myID + SingleMotorReplyIDOffset, MASK_ALL)){
        myCan->readMSGFrom(myID + SingleMotorReplyIDOffset, MASK_ALL);
    }

    //This is disgusting
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding[7];
    } myMsg = {0};

    myMsg.command = 0x92;

    //send the message
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));


    //Await response
    CANFrame recv;
    while (true){
        recv = myCan->readMSGFrom(myID + SingleMotorReplyIDOffset, MASK_ALL, 2000);
        
        //Check if we actually got a response
        if (recv.can_dlc == 0) {
            //return -1;
        }

        //Check if the command matches
        // printf("Got %02x\n",(uint8_t)recv.data[0]);
        if ((uint8_t)recv.data[0] == 0x92){
            break;
        } 
    }
    

    //Decode result
    int32_t result = 0;
    __dbg_dump((char*)&recv.data, sizeof(recv.data));
    memcpy(&result, recv.data + 4, sizeof(int32_t));
    //This is even worse
    return (double)result * 0.01;
}


void MyActuatorMotor::stop() {
    //Stop the motor
    //This is disgusting
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding[7];
    } myMsg = {0};

    myMsg.command = 0x81;

    //send the message
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));
}

void MyActuatorMotor::estop() {
    //Stop the motor
    //This is disgusting
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding[7];
    } myMsg = {0};

    myMsg.command = 0x80;

    //send the message
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));
}


void MyActuatorMotor::setVelocity(double vel) {
    //Set the motor to velocity mode
    //This is disgusting
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t torqueMax; //0-255
        uint8_t padding[2];
        int32_t speed; // Speed degrees per second/ 100
    } myMsg = {0};

    myMsg.command = 0xA2;
    myMsg.torqueMax = MaxTorque;
    myMsg.speed = (int32_t)(vel*100);

    //send the message
    __dbg_dump((char*)&myMsg, sizeof(myMsg));
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));
}


void MyActuatorMotor::setPosition(double pos) {
    //Set the position of the motor
    //This is disgusting
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding;
        uint16_t speedLimit;
        int32_t position;
    } myMsg = {0};

    myMsg.command = 0xA4;
    myMsg.speedLimit = SpeedLimit;
    myMsg.position = (int32_t)(pos*100);

    //send the message
    __dbg_dump((char*)&myMsg, sizeof(myMsg));
    myCan->writeMSG(myID + SingleMotorMsgIDOffset, (char*)&myMsg, sizeof(myMsg));
}

//The Tick function
void MyActuatorMotor::tick() {
    // Do nothing
}


MyActuatorMotor::~MyActuatorMotor() {
  //dtor
}