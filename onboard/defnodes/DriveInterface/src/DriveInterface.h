
#ifndef RAT3_DEMO_DRIVEINTERFACE_H
#define RAT3_DEMO_DRIVEINTERFACE_H

#include "ChannelLossycast.hpp"
#include <iostream>
#include "ControllerState.h"
#include "torque_handler.hpp"

class DriveInterface {
    private:
        TorqueHandler& torque_handler;
        LossyCastSubscriber subscriber;
    public:
        DriveInterface (
            TorqueHandler& torque_handler, const std::string& defcom_template, const char* can_interface
        );
        void run();
        void controlRoverDrive(ControllerState state); // Control rover wheels};
};
#endif //RAT3_DEMO_DRIVEINTERFACE_H
