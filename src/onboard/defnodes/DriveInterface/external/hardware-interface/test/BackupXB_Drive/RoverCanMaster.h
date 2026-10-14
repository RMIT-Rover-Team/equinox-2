#pragma once
#include "GenericCan.h"
#include "CommandUtils.h"
#include <iostream>
#include <utility>
#include <tuple>
#include <cstring>
#include <stdint.h>


#define MASTER_CAN_ID 0

struct ReceivedState {
    int motor_id;
    bool error_flag;
    bool uncallibrated_flag;
};

struct Datapoint {
    int stream_id;
    int channel_id;
};

namespace CommandByteLayout {
    constexpr uint8_t MOTOR_ID_POS = 4;
    constexpr uint8_t STATE_BIT_POS = 3;
}


class RoverCanMaster {
    private:
        GenericCan& can;
        uint8_t myID;
        uint8_t identifierCounter;

        uint16_t generateCanID(uint8_t source, uint8_t dest);
        ReceivedState convertReceivedState(CANFrame &receivedMSG);


    public:
        RoverCanMaster(GenericCan& myCan, uint8_t myID);

        bool EStop(uint8_t destID);
        bool Calibrate(uint8_t destID,int motor_id);
        ReceivedState SetMotorPosition(uint8_t destID, int motor_id, float position);
        ReceivedState SetMotorSpeed(uint8_t destID, int motor_id, float speed);
        ReceivedState ToggleState(uint8_t destID, int motor_id, bool toggle_state);
        std::pair<ReceivedState, float> GetMotorPosition(uint8_t destID, int motor_id);
        std::pair<ReceivedState, float> GetMotorSpeed(uint8_t destID, int motor_id);
        std::pair<Datapoint, float> BroadcastDataPoint();
        std::pair<Datapoint, float> RequestDataPoint(uint8_t destID,int stream_id, int channel_id);
        bool ping(uint8_t destID);


};