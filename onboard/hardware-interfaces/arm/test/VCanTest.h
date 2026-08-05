#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"
#include "../src/ArmPayload.h"



#define CANBufferLen 32

typedef struct {
    GroupId group_id;
    uint8_t device_id;
    CommandId cmd;
    char data[CanDataLength];
} CANData;

class VCANTest : public testing::Test {
protected:
    VCANTest();
    void SetUp();
    void read_from_vcan();

    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    ArmPayload payload;
    WrappedCANBus can_reader;

    CANData msgs[CANBufferLen];
    uint8_t msgs_recieved;
};