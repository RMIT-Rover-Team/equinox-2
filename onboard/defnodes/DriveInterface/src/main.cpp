#include "DriveInterface.h"

#define DEFCOM_PATH "COMFILES/DriveController.defcom"
#if defined(TORQUE_HANDLER_USE_MINIROVER_GPIO)
// The GPIO backend opens the Raspberry Pi's primary character-device chip.
#define HARDWARE_INTERFACE "/dev/gpiochip0"
#else
// The default backend opens the rover's SocketCAN network interface.
#define HARDWARE_INTERFACE "can0"
#endif

int main() {
    TorqueHandler torque_handler(HARDWARE_INTERFACE);
    DriveInterface drive_interface(
        torque_handler, DEFCOM_PATH, "can0"
    );

    drive_interface.run();
}
