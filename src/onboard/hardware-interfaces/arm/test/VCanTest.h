#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"
#include "../src/ArmPayload.h"



#define CANBufferLen 32

typedef struct {
    int group_id;
    uint8_t device_id;
    char cmd;
    bool is_myactuator_reply;
    char data[CanDataLength];
} CANData;

class VCANTest : public testing::Test {
protected:
    VCANTest();
    void SetUp();
    void read_from_vcan();

    WrappedCANBus can_bus;
    ArmPayload payload;
    WrappedCANBus can_reader;

    CANData msgs[CANBufferLen];
    uint8_t msgs_recieved;
};