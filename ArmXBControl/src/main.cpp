#include <iostream>
#include <chrono>
#include <thread>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>

const int ArmXBChannelID = 2;

// IK Config
const double posScale = 0.01;
const double angScale = 5;


//The Raised Position in degrees
const double jointRaised[6] = {
    0,
    140+90,
    130,
    -14,
    14,
    1.5
};

// The Stowed Position in degrees
const double jointStowed[6] = {
    0,
    -180,
    170,
    95,
    0,
    8
};

//Input Scales
const double jointScales[6] = {
    20,
    20,
    20,
    20,
    20,
    20
};

// The operation modes
enum class ArmMode : int {
    JointControl = 0,
    IKControl = 1,
    PresetControl = 2
};


float toRadians(float x){
    return (x/180.0)*3.141592;
}

// Handle Velocity Control of the Arm from a direct input
void directControl(LossyCastPublisher* myPub, MessageStructure* myControl){
    //New Message for outgoing
    MessageStructure myMessage = myPub->getNewMessageObject();

    

    // Analogue Mapped Joints

    // Base Joint LR
    myMessage.setFloat("jointSetValues",toRadians(myControl->getFloat("L_Stick_X")) * jointScales[0],0);
    // Base Up Down
    myMessage.setFloat("jointSetValues",toRadians(myControl->getFloat("L_Stick_Y")) * jointScales[1],1);
    // Elbow
    myMessage.setFloat("jointSetValues",toRadians(myControl->getFloat("R_Stick_Y")) * jointScales[2],2);
    // Spin
    myMessage.setFloat("jointSetValues",toRadians(myControl->getFloat("R_Stick_X")) * jointScales[4],5);

    // DPAD Mapped Joints

    // Wrist 1
    if (myControl->getChar("D_Pad_Up")){
        myMessage.setFloat("jointSetValues",toRadians(-1) * jointScales[3],3);
    }
    else if (myControl->getChar("D_Pad_Down")){
        myMessage.setFloat("jointSetValues",toRadians(1) * jointScales[3],3);
    }
    else {
        myMessage.setFloat("jointSetValues",0,3);
    }

    // Wrist 2
    if (myControl->getChar("D_Pad_Left")){
        myMessage.setFloat("jointSetValues",toRadians(-1) * jointScales[5],4);
    }
    else if (myControl->getChar("D_Pad_Right")){
        myMessage.setFloat("jointSetValues",toRadians(1) * jointScales[5],4);
    }
    else {
        myMessage.setFloat("jointSetValues",0,4);
    }

    // Set the message type to V for velocity
    myMessage.setBytes("movementType", (uint8_t*)"V");

    myPub->publish(myMessage);
}

double ikpos[3] = {0};
double ikang[3] = {0};

void ikControl(LossyCastPublisher* myPub, MessageStructure* myControl){
    //New Message for outgoing
    MessageStructure myMessage = myPub->getNewMessageObject();

    // Check if we are in angle or position mode
    //if (myControl->getChar("L_Stick_Button") || myControl->getChar("R_Stick_Button")){
    if (myControl->getFloat("L_Trigger") > -0.9){
        printf("IK Angle Mode\n");

        // Axis are 0, 1, 3, 4
        ikang[2] += myControl->getFloat("L_Stick_Y") * angScale * -1;
        ikang[0] += myControl->getFloat("R_Stick_X") * angScale * 1;
        ikang[1] += myControl->getFloat("R_Stick_Y") * angScale * -1;
    }
    else {
        printf("IK Position Mode\n");
        for (int i = 0; i < 3; i++){
            ikpos[2] += myControl->getFloat("L_Stick_Y") * posScale * -1;
            ikpos[0] += myControl->getFloat("R_Stick_X") * posScale * 1;
            ikpos[1] += myControl->getFloat("R_Stick_Y") * posScale * -1;
        }
    }

    // Send the message
    for (int i = 0; i < 3; i++){
        myMessage.setFloat("endPosition",ikpos[i],i);
        myMessage.setFloat("endOrientation",toRadians(ikang[i]),i);

        printf("IK Index %i Angle %f Pos %f\n",i,ikang[i],ikpos[i]);
    }

    myPub->publish(myMessage);
}
    

void presetControl(LossyCastPublisher* myPub, MessageStructure* myControl){
    //New Message for outgoing
    MessageStructure myMessage = myPub->getNewMessageObject();
    char messageMode = 0; // A flag so we can ignore the movement if not important

    // If DPad Up is pressed, we go into raised mode
    if (myControl->getChar("D_Pad_Up")){
        // Copy the positions into the message
        for (int i = 0; i < 6; i++){
            myMessage.setFloat("jointSetValues",toRadians(jointRaised[i]),i);
        }
        messageMode = 'P';
    }

    // If DPad Down is pressed, we go into stowed mode
    else if (myControl->getChar("D_Pad_Down")){
        // Copy the positions into the message
        for (int i = 0; i < 6; i++){
            myMessage.setFloat("jointSetValues",toRadians(jointStowed[i]),i);
        }
        messageMode = 'P';
    }

    // Zero all values
    else {
        messageMode = 'V';
        for (int i = 0; i < 6; i++){
            myMessage.setFloat("jointSetValues",0,i);
        }
    }

    // Send the message
    myMessage.setBytes("movementType", (uint8_t*)&messageMode);
    myPub->publish(myMessage);
}

// Asynchronously tells the backend what the state of the arm control is
void statusWorker(ArmMode* IKMode){
    //Start the Status Publisher
    LossyCastPublisher myStatusCast("COMFILES/ArmControlStatus.defcom");

    while (true){
        // Announce the current state of arm control
        MessageStructure myStatusMessage = myStatusCast.getNewMessageObject();
        myStatusMessage.setChar("inputState", (int)(*IKMode));
        myStatusCast.publish(myStatusMessage);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char *argv[]) {
    // Set Up Controller Listener
    LossyCastSubscriber controllerSub("COMFILES/XBox.defcom");
    printf("Controller Subscriber Initialised\n");
    
    //Start the Arm Control Publishers
    LossyCastPublisher myCast("COMFILES/ArmControl.defcom");
    LossyCastPublisher myIKCast("COMFILES/ArmIKControl.defcom");
    printf("Outbound Control Channels Up\n");

    // Config Variables
    ArmMode mode = ArmMode::JointControl;

    // Start the status worker
    std::thread statusThread(statusWorker, &mode);

    while (true){
        // Get a controller message
        MessageStructure controlMessage = controllerSub.subscribe();

        // Make sure it is an arm message
        if (controlMessage.getInt("Channel_ID") != ArmXBChannelID){
            continue; // Skip everything else
        }

        //If X Button, then switch to IK
        if (controlMessage.getChar("X_Button")){
            printf("Enter IK Mode\n");
            mode = ArmMode::IKControl;
        }

        // If Y Button is pressed, switch to user velocity
        if (controlMessage.getChar("Y_Button")){
            printf("Enter User Velocity Mode\n");
            mode = ArmMode::JointControl;
        }

        // If B button is pressed, we go to preset mode
        if (controlMessage.getChar("B_Button")){
            printf("Enter Preset Mode\n");
            mode = ArmMode::PresetControl;
        }

        if (mode == ArmMode::IKControl){
            ikControl(&myIKCast, &controlMessage);
        }
        else if (mode == ArmMode::PresetControl) {
            presetControl(&myCast, &controlMessage);
        }
        else {
            directControl(&myCast, &controlMessage);
        }

        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        
    }
    return 0;
}