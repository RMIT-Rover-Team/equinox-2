
#ifndef RAT3_DEMO_CONTROLLERSTATE_H
#define RAT3_DEMO_CONTROLLERSTATE_H
#include <cstdint>
#include <iomanip>
#include "FlexibleMessageStructure.hpp"
#include "ChannelMulticast.hpp"

struct ControllerState {
    float right_stick;
    float left_stick;
    bool right_bumper;
    bool dpad_up;
    bool dpad_down;
    bool dpad_left;
    bool dpad_right;

    ControllerState();
    ControllerState(MessageStructure& message);
    void printLabel(std::string label);
    void dump();
};

#endif
