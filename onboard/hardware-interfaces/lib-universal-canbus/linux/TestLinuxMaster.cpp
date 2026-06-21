#include <iostream>
#include "../libuniversalcan/RoverCanMaster.h"
#include "../libuniversalcan/SocketCanWrapper.h"

int main(){
    WrappedCANBus myCan("vcan0");
    printf("Init Ratcan\n");

    RoverCanMaster myMaster(myCan, MASTER_CAN_ID);
    printf("Init Master\n");

    const int destSlave = 2;

    //Perform a test of each method

    printf("Ping First!\n");
    printf("Result: %i\n",myMaster.ping(destSlave));


    printf("Test ESTOP\n");
    myMaster.EStop(destSlave);

    printf("Test Calibrate\n");
    myMaster.Calibrate(destSlave, 3);

    printf("Test Set Position\n");
    myMaster.SetMotorPosition(destSlave, 4, 100);

    printf("Test Set Speed\n");
    myMaster.SetMotorSpeed(destSlave, 5, -200);

    printf("Test Toggle\n");
    myMaster.ToggleState(destSlave, 6, 1);
    myMaster.ToggleState(destSlave, 7, 0);

    std::pair<ReceivedState, float> response;
    printf("Test Get Position\n");
    response = myMaster.GetMotorPosition(destSlave, 8);
    printf("Position: %lf\n", response.second);

    printf("Test Get Speed\n");
    response = myMaster.GetMotorSpeed(destSlave, 9);
    printf("Speed: %lf\n", response.second);

}