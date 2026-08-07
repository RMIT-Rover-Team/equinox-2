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
            
                struct __attribute__((packed)) CAN_Message {
                    uint8_t command;
                    uint8_t padding[3];
                    int32_t position;
                } response_data = {0};

                response_data.command = 0x92;
                response_data.position = (int32_t)(100*positions[i]);
                
                std::cout << "responding to req for motor " << i << std::endl;
                std::printf("%x %d\n", response_data.command, response_data.position);
                
                can_bus.writeMSG(0x0 + SingleMotorReplyIDOffset, (char*)&response_data, sizeof(response_data));
                
                // loop around and wait for next response
                // because get_position is blocking, it will be a while until the next request
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    return 0;
}