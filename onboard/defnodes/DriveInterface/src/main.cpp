#include "DriveInterface.h"

#define DEFCOM_PATH "COMFILES/DriveController.defcom"
#define CAN_INTERFACE "can0"

int main() {
    TorqueHandler torque_handler(CAN_INTERFACE);
    DriveInterface drive_interface(
        torque_handler, DEFCOM_PATH, "can0"
    );

    drive_interface.run();
}