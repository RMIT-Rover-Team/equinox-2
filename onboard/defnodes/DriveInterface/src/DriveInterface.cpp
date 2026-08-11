#include "DriveInterface.h"

DriveInterface::DriveInterface (
    TorqueHandler& torque_handler,
    const std::string& defcom_template,
    const char* can_interface
) : torque_handler(torque_handler), subscriber(defcom_template) {}

void DriveInterface::run() {
    torque_handler.setMode(TorqueDriveMode::UNLOCKED_VELOCITY);
    torque_handler.setSpeed(0,0);
    torque_handler.enable();
    try {
        std::cout << "Listening for controller input from DEFCOM...\n";

        while (true) {
            MessageStructure message = subscriber.subscribe();

            if (message.totalSize != 0) {
                ControllerState state(message);

                // Calculate torque values and call torque_handler
                controlRoverDrive(state);            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void DriveInterface::controlRoverDrive(ControllerState state) {
    torque_handler.setMode(state.right_bumper ?
        TorqueDriveMode::LOCKED_VELOCITY : TorqueDriveMode::UNLOCKED_VELOCITY);
    torque_handler.setSpeed(state.left_stick, state.right_stick);
}

