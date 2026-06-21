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

// ping
// estop
// txint8
// txint16
// txfloat
// txdata

RoverCanMaster::RoverCanMaster(GenericCan& my_can, uint8_t my_id)
  : can(my_can), my_id(my_id) {
}


uint16_t RoverCanMaster::generate_header(uint8_t group, uint8_t device, uint8_t command){
  uint16_t header = 0;
  
  header |= (group & 0x3) << 9;
  header |= (device & 0x1f) << 4;
  header |= (command & 0xf);
  
  return header;
}

void RoverCanMaster::ping(uint8_t group, uint8_t device) {
  printf("Ping\n");
  Command myCommand;

  // WRITE MSG
  uint16_t canbus_header = generate_header(group, device, CommandId::PING);

  can.writeMSG(canbus_header, 0, 0);
}

void RoverCanMaster::estop(uint8_t group, uint8_t device) {
  printf("Estop\n");
  Command myCommand;

  // WRITE MSG
  uint16_t canbus_header = generate_header(group, device, CommandId::ESTOP);
  can.writeMSG(canbus_header, 0, 0);
}   

void RoverCanMaster::tx_int8(uint8_t group, uint8_t device, int8_t numbers[8]) {
  printf("Tx_int8");
  Command myCommand;

  uint16_t canbus_header = generate_header(group, device, CommandId::TXINT8);

  for (int i = 0; i < 8; i++) {
    myCommand.addInt8(numbers[i]);
  }

  can.writeMSG(canbus_header, (char*)myCommand.getBuffer(), myCommand.getBufferLength());
}

void RoverCanMaster::tx_int16(uint8_t group, uint8_t device, int16_t numbers[4]) {
  printf("Tx_int16");
  Command myCommand;

  uint16_t canbus_header = generate_header(group, device, CommandId::TXINT16);

  for (int i = 0; i < 4; i++) {
    myCommand.addInt16(numbers[i]);
  }

  can.writeMSG(canbus_header, (char*)myCommand.getBuffer(), myCommand.getBufferLength());
}

void RoverCanMaster::tx_float(uint8_t group, uint8_t device, float numbers[4]) {
  printf("Tx_float");
  Command myCommand;

  uint16_t canbus_header = generate_header(group, device, CommandId::TXFLOAT);

  for (int i = 0; i < 2; i++) {
    myCommand.addFloat(numbers[i]);
  }

  can.writeMSG(canbus_header, (char*)myCommand.getBuffer(), myCommand.getBufferLength());
}

void RoverCanMaster::tx_data(uint8_t group, uint8_t device, uint8_t data[8]) {
  printf("Tx_data");
  Command myCommand;
  
  uint16_t canbus_header = generate_header(group, device, CommandId::TXDATA);

  for (int i = 0; i < 8; i++) {
    myCommand.addUInt8(data[i]);
  }

  can.writeMSG(canbus_header, (char*)myCommand.getBuffer(), myCommand.getBufferLength());
}