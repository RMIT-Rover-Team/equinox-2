
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "simplecontroller/RemoteController.h"
#include "RoverCanMaster.h"
#include "SocketCanWrapper.h"
#include "GenericCan.h"
#include <string.h>
#include <limits>
#include <chrono>
#include <thread>
#include <math.h>
#include "torque_handler.hpp"

#define IMU_ID 5
#define MAX_BAZ 180
#define MIN_BAZ 80

int main(int argc, char **argv){
    if (argc < 2){
        std::cout << "Usage:" << std::endl;
        std::cout << argv[0] << " <CANBUS INTERFACE NAME>" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Init Joystick
    RemoteController myControl(8011);

    //Init CAN
    GenericCan* myCan = new WrappedCANBus(argv[1]);

    //Create a torque handler
    //In test mode the can interface is ignored
    TorqueHandler* myHandler = new TorqueHandler(argv[1]);
    printf("Init handler\n");
    myHandler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);

    int targetCanID = IMU_ID;

    std::cout << "Connected to IMU ID 0x" << targetCanID << " With ID 0x" << MASTER_CAN_ID << std::endl;

    //Prepare Master
    RoverCanMaster myMaster(*myCan, 0);
    
    int run = 1;
    double leftSpeed, rightSpeed;
    leftSpeed = 0;
    rightSpeed = 0;

    int upd = 0;
    int currentMode = 0;

    printf("Awaiting Enable button......");
    while (true){
        myControl.tick();

        if (myControl.buttonAvailable(3)){
            myControl.getButton(3);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf("Enabling\n");
    myHandler->enable();
    printf("We are ready to drive\n");

    while (run){
        myControl.tick();
        
       
        if (myControl.axisAvailable(1)){
            //Base Up down
            leftSpeed = MAX_BAZ*(double)myControl.getAxis(1) / 32768.0;
            upd = 1;
        }
        
        if (myControl.axisAvailable(4)){
            //Elbow
            rightSpeed = MAX_BAZ*(double)myControl.getAxis(4) / 32768.0;
            upd = 1;
        }



        if (upd){
            upd = 0;
            //Make sure we have suitable power
            if (((fabs(leftSpeed) > MIN_BAZ) && (fabs(rightSpeed) > MIN_BAZ))){
                printf("New BAZ Speed: %lf %lf ",leftSpeed,rightSpeed);
                if ((leftSpeed != 0) && (rightSpeed != 0)){
                    //myMaster.ToggleState(IMU_ID, 1, 1);
                    printf("LED: MOTION\n");
                }
                else {
                    //myMaster.ToggleState(IMU_ID, 0, 1);
                    printf("LED: SAFE\n");
                }

                //Set wheel speeds
                myHandler->setSpeed(leftSpeed,rightSpeed);

                OdomReading myReading = myHandler->getOdom();
                printf("Current Odom - Speed: %f %f Pos: %f %f\n",myReading.leftSpeed, myReading.rightSpeed, myReading.leftPos, myReading.rightPos);
            }
            else {
                myHandler->setSpeed(0,0);
                //myMaster.ToggleState(IMU_ID, 0, 1);
                printf("MOTION STOP - LED: SAFE\n");
            }
        }
        






        if (myControl.buttonAvailable(6)){
            run = 0;
        }

        //Enable/Disable
        if (myControl.buttonAvailable(2)){
            if (myControl.getButton(2)){
                printf("Disabling...");
                myHandler->disable();
            }
        }
        if (myControl.buttonAvailable(3)){
            if (myControl.getButton(3)){
                printf("Enable and Clear Errors...");
                myHandler->enable();
            }
        }

        //Switch Drive mode
        if (myControl.buttonAvailable(0)){
            if (myControl.getButton(0)){
        
                if (currentMode){
                    currentMode = 0;
                    printf("Switch to Unlocked Mode\n");
                    myHandler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);
                }
                else {
                    currentMode = 1;
                    printf("Switch to LOCKED Mode\n");
                    myHandler->setMode(TorqueDriveMode::LOCKED_VELOCITY);
                }   
            }
        }





        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    //myMaster.ToggleState(IMU_ID, 0, 1);
    printf("LED: SAFE\n");

    delete myCan;


    return 0;
}
