#include "torque_handler.hpp"
#include <iostream>

#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h> 

#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"
#include "TCPstreams.h"

#define IMU_ID 5




int main(int argc, char** argv){
    if (argc < 2){
        printf("Usage: %s <canInterface>\n",argv[0]);
        exit(0);
    }

    int myServer = openserver("0.0.0.0",9000);
    printf("Init Server");

    //Accept a connection
    int myCon = taccept(myServer);

    //In test mode the can interface is ignored
    TorqueHandler* myHandler = new TorqueHandler(argv[1]);
    printf("Init handler\n");

    //Create a can connection
    WrappedCANBus* myBus = new WrappedCANBus(argv[1]);
    printf("Init SocketCan\n");

    RoverCanMaster* myMaster = new RoverCanMaster(*myBus,0);
    printf("Init Indicator\n");

    printf("Calibrate\n");
    //myHandler->calibrate();
    //std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("Enable\n");
    myHandler->enable();
    myHandler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);

    int run = 1;
    float leftsp = 0;
    float rightsp = 0;

    //Safe 0
    //Auto Prep 2
    //Auto 3

    union convBuf {
        char buffer[16];
        double leftRight[2];
    } myBuf;

    myMaster->ToggleState(IMU_ID, 0, 1);
    while (run){
        read(myCon, myBuf.buffer, 16);
        printf("Speed to %lf %lf\n",myBuf.leftRight[0],myBuf.leftRight[1]);
        myHandler->setSpeed(myBuf.leftRight[0],myBuf.leftRight[1]);

        

        if ((leftsp != 0) && (rightsp != 0)){
            myMaster->ToggleState(IMU_ID, 3, 1);
        }
        else {
            myMaster->ToggleState(IMU_ID, 2, 1);
        }

        //Show current speeds
        OdomReading myReading = myHandler->getOdom();
        printf("Current Odom - Speed: %f %f Pos: %f %f\n",myReading.leftSpeed, myReading.rightSpeed, myReading.leftPos, myReading.rightPos);
    }

    printf("Clean Up\n");
    delete myHandler;
    delete myMaster;
    delete myBus;
    printf("Done\n");

}
