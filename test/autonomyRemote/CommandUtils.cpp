#include "CommandUtils.h"
#include <stdint.h>

//Debug
/*void dumpBufferCM(unsigned char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        printf("%02x ", (uint8_t)buffer[i]);
    }
    printf("\n");
  }*/


//Constructor, called when object is created
Command::Command(){
  //Clear the buffer
  this->clearCommand();
}

//Destructor, called when object is destroyed
Command::~Command(){

}

//Gets the buffer (For sending and receiving)
unsigned char* Command::getBuffer(){
  return buffer;
}

//Gets the buffer length
int Command::getBufferLength(){
  return maximumCommandLength;
}

/* 
  The following commands are used to build a command
*/

//adds a float
void Command::addFloat(float value){
  cF.value = value;
  for (int i = 0; i < sizeof(float); i++) {
    buffer[dataIndex+i] = cF.data[i];
  }
  dataIndex += sizeof(float);
}

//adds a uint8
void Command::addUInt8(uint8_t value){
  cU8.value = value;
  
  for (int i = 0; i < sizeof(uint8_t); i++) {
    buffer[dataIndex+i] = cU8.data[i];
  }

  dataIndex += sizeof(uint8_t);
}

//adds a uint16
void Command::addUInt16(uint16_t value){
  cU16.value = value;
  
  for (int i = 0; i < sizeof(uint16_t); i++) {
    buffer[dataIndex+i] = cU16.data[i];
  }

  dataIndex += sizeof(uint16_t);
}

//Adds a Boolean
void Command::addBool(bool value){
  cB.value = value;

  for (int i = 0; i < sizeof(bool); i++) {
      buffer[dataIndex+i] = cB.data[i];
  }

  dataIndex += sizeof(bool);
}

/*
  The following commands are used to read back a command
*/


//Gets the next float
float Command::getNextFloat(){
  //Read the bytes from dataIndex to dataIndex+sizeof(double) and then convert back to double
  for(int i = 0; i < sizeof(float); i++){
    cF.data[i] = buffer[dataIndex + i];
  }

  dataIndex += sizeof(float);
  return cF.value;
}

//Gets the next uint8
uint8_t Command::getNextUint8(){
  //Read the bytes from dataIndex to dataIndex+sizeof(uint8_t) and then convert back to uint8
  for(int i = 0; i < sizeof(uint8_t); i++){
    cU8.data[i] = buffer[dataIndex +i];
  }
  dataIndex += sizeof(uint8_t);
  return cU8.value;
}

//Gets the next uint16
uint16_t Command::getNextUint16(){
  //Read the bytes from dataIndex to dataIndex+sizeof(uint8_t) and then convert back to uint8
  for(int i = 0; i < sizeof(uint16_t); i++){
    cU16.data[i] = buffer[dataIndex +i];
  }
  dataIndex += sizeof(uint16_t);
  return cU16.value;
}

//Gets the next bool
bool Command::getNextBool(){
  //Read the bytes from dataIndex to dataIndex+sizeof(bool) and then convert back to bool
  for(int i = 0; i < sizeof(bool); i++){
    cB.data[i] = buffer[dataIndex +i];
  }
  dataIndex += sizeof(bool);
  return cB.value;
}



//Clears the command buffer
void Command::clearCommand(){
  for (int index = 0; index < maximumCommandLength; index++){
    buffer[index] = 0;
  }
  dataIndex = dataStartIndex;
}
