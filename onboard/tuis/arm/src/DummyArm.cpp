// responds to myactuator get_position requests for when testing the TUI
#include "SocketCanWrapper.h"
#include "MyActuatorMotor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <cstring>

int main() {
    WrappedCANBus can_bus("vcan0");

    const double SNAP_DISTANCE = 0.1;
    std::array<double, 6> target_positions = {0};
    std::array<double, 6> positions = {0};
    

    while (true) {
        // update positions towards targets
        for (int i = 0; i < 6; ++i) {
            if (target_positions[i] > positions[i] + SNAP_DISTANCE) positions[i] += SNAP_DISTANCE;
            else if (target_positions[i] < positions[i] - SNAP_DISTANCE) positions[i] -= SNAP_DISTANCE;
            else if (target_positions[i] != positions[i]) positions[i] = target_positions[i];
        }

        // send responses
        for (int i = 0; i < 6; ++i) {
            if (can_bus.availableFrom(i + SingleMotorMsgIDOffset, MASK_ALL)) {
                CANFrame msg = can_bus.readMSGFrom(i + SingleMotorMsgIDOffset, MASK_ALL, 100);
                uint8_t cmd = msg.data[0];
                if (cmd != 0x92) {
                    std::printf("Non getpos msg recieved 0x%02X\n", cmd);
                    continue;
                }

                struct __attribute__((packed)) CAN_Message {
                    uint8_t command;
                    uint8_t padding[3];
                    int32_t position;
                } response_data = {0};

                response_data.command = 0x92;
                response_data.position = (int32_t)(100*positions[i]);

                std::printf("res motor %d pos %d\n", i, response_data.position);
                can_bus.writeMSG(i + SingleMotorReplyIDOffset, (char*)&response_data, sizeof(response_data));
            
                // get_position is blocking
                // send our response, then wait for the next request
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    return 0;
}