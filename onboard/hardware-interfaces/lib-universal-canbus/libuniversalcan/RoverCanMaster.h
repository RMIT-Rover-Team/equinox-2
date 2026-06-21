#pragma once
#include "GenericCan.h"
#include "CommandUtils.h"
#include <iostream>
#include <utility>
#include <tuple>
#include <cstring>
#include <stdint.h>

enum Payload
{
  ARM_END_EFFECTOR,
  SCIENCE,
  EXCAVATOR,
  GIMBAL,
  PDB,
};

enum GroupId
{
  ONBOARD = 0x0,
  PAYLOAD = 0x2,
};

enum CommandId
{
  PING = 0x0,
  ESTOP = 0x1,
  TXINT8 = 0x2,
  TXINT16 = 0x3,
  TXFLOAT = 0x4,
  TXDATA = 0x5,
};

namespace CommandByteLayout
{
  constexpr uint8_t MOTOR_ID_POS = 4;
  constexpr uint8_t STATE_BIT_POS = 3;
}

class RoverCanMaster
{
public:
  /// @brief Constructs the CAN Master
  /// @param myCan Appropriate CAN Interface. If on Linux, use SocketCanWrapper. If on embedded, use EQUCAN.
  /// @param myID The Master's CAN id
  RoverCanMaster(GenericCan &myCan, uint8_t myID);

  void ping(uint8_t group, uint8_t device);
  void estop(uint8_t group, uint8_t device);
  void tx_int8(uint8_t group, uint8_t device, int8_t[8]);
  void tx_int16(uint8_t group, uint8_t device, int16_t integers[4]);
  void tx_float(uint8_t group, uint8_t device, float integers[4]);
  /// @brief Send an arbitrary byte buffer
  /// @param group Group of intended device
  /// @param device Identifier of intended device
  /// @param data Arbitrary byte buffer
  void tx_data(uint8_t group, uint8_t device, uint8_t data[8]);

private:
  GenericCan &can;
  uint8_t my_id;

  /// @brief Generates a CAN Bus header.
  /// @param group The group ID of the device
  /// @param device The device ID of the device
  /// @param command The command to be used
  /// @return Returns the header frame
  uint16_t generate_header(uint8_t group, uint8_t device, uint8_t command);
};