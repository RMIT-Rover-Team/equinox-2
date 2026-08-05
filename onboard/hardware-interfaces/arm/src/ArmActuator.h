#pragma once

#include <stdint.h>
#include "GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

enum MyActuatorCommand {
    ReadEncoderPosition = 0x60
};

/// @brief MyActuator wrapper
class ArmActuator {
private:
    uint8_t motor_id;
    int16_t velocity;
    RoverCanMaster &can_master;

public:
    ArmActuator(uint8_t motor_id, RoverCanMaster &can_master);
    ~ArmActuator();

    void set_velocity(int16_t target_velocity);
    void set_position(int16_t target_position);
    int32_t get_encoded_position();
};
