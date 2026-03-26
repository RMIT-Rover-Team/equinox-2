
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

// Standard colors
#define RESET       "\033[0m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

// Bold versions
#define BOLD        "\033[1m"
#define BOLD_RED    "\033[1;31m"
#define BOLD_GREEN  "\033[1;32m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_BLUE   "\033[1;34m"

// Inverted (reverse video)
#define INV_BLACK   "\033[7;30m"
#define INV_RED     "\033[7;31m"
#define INV_GREEN   "\033[7;32m"
#define INV_YELLOW  "\033[7;33m"
#define INV_BLUE    "\033[7;34m"
#define INV_MAGENTA "\033[7;35m"
#define INV_CYAN    "\033[7;36m"
#define INV_WHITE   "\033[7;37m"


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

    printf(INV_YELLOW "Awaiting Enable button......" RESET);
    while (true){
        myControl.tick();

        if (myControl.buttonAvailable(3)){
            myControl.getButton(3);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf(INV_GREEN "Enabling\n" RESET);
    myHandler->enable();
    printf(INV_GREEN "We are ready to drive\n" RESET);

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
                printf(INV_CYAN "MOTION STOP - LED: SAFE\n" RESET);
            }
        }
        






        if (myControl.buttonAvailable(6)){
            run = 0;
        }

        //Enable/Disable
        if (myControl.buttonAvailable(2)){
            if (myControl.getButton(2)){
                printf(INV_RED "Disabling..." RESET);
                myHandler->disable();
            }
        }
        if (myControl.buttonAvailable(3)){
            if (myControl.getButton(3)){
                printf(INV_GREEN "Enable and Clear Errors..." RESET);
                myHandler->enable();
            }
        }

        //Switch Drive mode
        if (myControl.buttonAvailable(0)){
            if (myControl.getButton(0)){
        
                if (currentMode){
                    currentMode = 0;
                    printf(INV_MAGENTA "Switch to Unlocked Mode\n" RESET);
                    myHandler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);
                }
                else {
                    currentMode = 1;
                    printf(INV_MAGENTA "Switch to LOCKED Mode\n" RESET);
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
