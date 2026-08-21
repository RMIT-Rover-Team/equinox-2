// responds to myactuator get_position requests for when testing the TUI
#include "SocketCanWrapper.h"
#include "MyActuatorMotor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <cstring>



void set_position(std::array<double, 6> &target_positions, size_t i, const CANFrame &msg);
void get_position(const std::array<double, 6> &positions, size_t i, GenericCan &can_bus);



int main() {
    WrappedCANBus can_bus("vcan0");

    const double SNAP_DISTANCE = 0.1;
    std::array<double, 6> target_positions = {0};
    std::array<double, 6> positions = {0};

    while (true) {
        // update positions towards targets
        for (int i = 0; i < 6; ++i) {
            if (std::abs(target_positions[i] - positions[i]) < 2 * SNAP_DISTANCE) positions[i] = target_positions[i];
            else if (positions[i] < target_positions[i]) positions[i] += SNAP_DISTANCE;
            else if (positions[i] > target_positions[i]) positions[i] -= SNAP_DISTANCE;
        }

        // send responses
        for (int i = 0; i < 6; ++i) {
            if (can_bus.availableFrom(i + SingleMotorMsgIDOffset, MASK_ALL)) {
                CANFrame msg = can_bus.readMSGFrom(i + SingleMotorMsgIDOffset, MASK_ALL, 100);
                uint8_t cmd = msg.data[0];

                if (cmd == 0xA4) {
                    set_position(target_positions, i, msg);
                    break;
                } else if (cmd == 0x92) {
                    get_position(positions, i, can_bus);
                    // get_position is blocking
                    // send our response, then wait for the next request
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                } else {
                    std::printf("Unknown msg recieved 0x%02X\n", cmd);
                    continue;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}

void set_position(std::array<double, 6> &target_positions, size_t i, const CANFrame &msg) {
    // position in last 4 bytes of data
    int32_t angle = 0;
    angle |= (int32_t)((uint8_t)msg.data[4]);
    angle |= (int32_t)((uint8_t)msg.data[5] << 8);
    angle |= (int32_t)((uint8_t)msg.data[6] << 16);
    angle |= (int32_t)((uint8_t)msg.data[7] << 24);

    target_positions.at(i) = ((double)angle) / 100.0;
    std::cout << "set motor " << i << " to: " << angle << std::endl;
}

void get_position(const std::array<double, 6> &positions, size_t i, GenericCan &can_bus) {
    struct __attribute__((packed)) CAN_Message {
        uint8_t command;
        uint8_t padding[3];
        int32_t position;
    } response_data = {0};

    response_data.command = 0x92;
    response_data.position = (int32_t)(100*positions[i]);

    std::printf("res motor %ld pos %d\n", i, response_data.position);
    can_bus.writeMSG(i + SingleMotorReplyIDOffset, (char*)&response_data, sizeof(response_data));
}
