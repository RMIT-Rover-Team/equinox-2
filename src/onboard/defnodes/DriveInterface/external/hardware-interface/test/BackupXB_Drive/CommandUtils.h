#pragma once
#include <stdint.h>

#define maximumCommandLength 8
#define dataStartIndex 0;


//Command Structure
// Byte 0-1 - Return Address
// Byte 2 - Command ID
// Byte 3-7 - Data

//Builds command, returns length of command written to buffer

union convertU8 {
  uint8_t value;
  unsigned char data[sizeof(uint8_t)];
};

union convertU16 {
  uint16_t value;
  unsigned char data[sizeof(uint16_t)];
};

union convertFloat {
  float value;
  unsigned char data[sizeof(float)];
};

union convertU32 {
  uint32_t value;
  unsigned char data[sizeof(uint32_t)];
};

//Kaelan should check
union convertBool {
  bool value;
  unsigned char data[sizeof(bool)];
};


class Command {
  private:
    unsigned char buffer[maximumCommandLength];
    int dataIndex = dataStartIndex;

    //The conversion unions
    union convertU8 cU8;
    union convertU32 cU32;
    union convertU16 cU16;
    union convertBool cB;
    union convertFloat cF;


  public:
    //Constructor, called when object is created
    Command();

    //Destructor, called when object is destroyed
    ~Command();

    //Gets the buffer (For sending and receiving)
    unsigned char* getBuffer();

    //Gets the buffer length
    int getBufferLength();

    //Clears the command buffer
    void clearCommand();

    /* 
      The following commands are used to build a command
    */


    //adds a float
    void addFloat(float value);

    //adds a uint8
    void addUInt8(uint8_t value);

    //adds a uint16
    void addUInt16(uint16_t value);

    //Adds a Boolean
    void addBool(bool value);

    /*
      The following commands are used to read back a command
    */

    //Gets the next float
    float getNextFloat();

    //Gets the next uint8
    uint8_t getNextUint8();

    //Gets the next uint16
    uint16_t getNextUint16();

    //Gets the next bool
    bool getNextBool();

};