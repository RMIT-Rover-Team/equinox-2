#include <iostream>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
#include "torque_handler.hpp"
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>

int main(int argc, char *argv[]) {
    if (argc < 2){
        std::cout << "Usage:" << argv[0] << " <CanbusInterface>" << std::endl;
        return 1;
    }

    std::cout << "Starting Drive on " << argv[1] << std::endl;
    

    // Initialise the torque handler
    TorqueHandler handler(argv[1]);
    handler.setMode(TorqueDriveMode::UNLOCKED_VELOCITY);

    handler.enable();
    handler.setSpeed(0,0);
    
    

    // Start Listening
    LossyCastSubscriber control("COMFILES/Drive.defcom");

    std::cout << "Begin Drive" << std::endl;

    int lastEnable = 1;
    while (true){
        MessageStructure msg = control.subscribe();

        if (msg.getChar("EStop")){
            std::cout << "EStop!" << std::endl;
            handler.estop();
        }
        else if (msg.getChar("Calibrate")){
            std::cout << "Calibrate!" << std::endl;
            handler.calibrate();
        }

        // Handle swapping
        if ((msg.getChar("Enable") == 1) && (lastEnable == 0)){
            std::cout << "Enable!" << std::endl;
            handler.enable();
            lastEnable = 1;
        }
        else if ((msg.getChar("Enable") == 0) && (lastEnable == 1)){
            std::cout << "Disable!" << std::endl;
            handler.disable();
            lastEnable = 0;
        }

        // Set the Drive Mode
        handler.setMode((TorqueDriveMode)msg.getChar("DriveMode"));

        // Set the Speed
        handler.setSpeed(msg.getFloat("LeftSpeed"), msg.getFloat("RightSpeed"));

    }

    return 0;
}