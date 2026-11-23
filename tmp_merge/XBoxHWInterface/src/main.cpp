#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
#include "SimpleController.h"

// Stick Drift Reject Value
const double stickDriftReject = 0.05;

double filterDrift(double value){
    if (fabs(value) < stickDriftReject){
        return 0;
    }
    return value;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./XBoxHWInterface ChannelNumber\n");
        return 0;
    }

    int channel = std::stoi(argv[1]);

    // Set Up Controller
    SimpleController myControl(16,16);
    printf("Controller Ready Channel %i\n",channel);
    
    //Start the Publisher
    LossyCastPublisher myCast("COMFILES/XBox.defcom");


    double convSpeed;
    int IKMode = 0;
    while (true){
        myControl.tick();

        // Make a new message
        MessageStructure myMessage = myCast.getNewMessageObject();

        // Set Channel
        myMessage.setInt("Channel_ID",channel);

        // Get each of the important buttons / axis
        myMessage.setFloat("L_Stick_X", filterDrift((double)myControl.getAxis(0) / 32768.0));
        myMessage.setFloat("L_Stick_Y", filterDrift((double)myControl.getAxis(1) / 32768.0));
        myMessage.setFloat("R_Stick_X", filterDrift((double)myControl.getAxis(3) / 32768.0));
        myMessage.setFloat("R_Stick_Y", filterDrift((double)myControl.getAxis(4) / 32768.0));

        myMessage.setFloat("L_Trigger", filterDrift((double)myControl.getAxis(2) / 32768.0));
        myMessage.setFloat("R_Trigger", filterDrift((double)myControl.getAxis(5) / 32768.0));

        myMessage.setChar("A_Button", (char)myControl.getButton(0));
        myMessage.setChar("B_Button", (char)myControl.getButton(1));
        myMessage.setChar("X_Button", (char)myControl.getButton(2));
        myMessage.setChar("Y_Button", (char)myControl.getButton(3));

        myMessage.setChar("Select_Button", (char)myControl.getButton(4));
        myMessage.setChar("Start_Button",  (char)myControl.getButton(6));

        myMessage.setChar("L_Stick_Button", (char)myControl.getButton(7));
        myMessage.setChar("R_Stick_Button", (char)myControl.getButton(8));

        myMessage.setChar("D_Pad_Up", (char)myControl.getButton(11));
        myMessage.setChar("D_Pad_Down", (char)myControl.getButton(12));
        myMessage.setChar("D_Pad_Left", (char)myControl.getButton(13));
        myMessage.setChar("D_Pad_Right", (char)myControl.getButton(14));

        myMessage.setChar("L_Bumper", (char)myControl.getButton(9));
        myMessage.setChar("R_Bumper", (char)myControl.getButton(10));
        

        myCast.publish(myMessage);        

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}