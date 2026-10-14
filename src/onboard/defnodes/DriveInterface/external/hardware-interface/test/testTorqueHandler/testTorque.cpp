#include "torque_handler.hpp"
#include <iostream>

#include <thread>
#include <chrono>

//Put it in test mode
#define TorqueHandler_TestMode 1

int main(){
    //In test mode the can interface is ignored
    TorqueHandler* myHandler = new TorqueHandler("vcan0");
    printf("Init\n");

    printf("Calibrate\n");
    myHandler->calibrate();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    printf("Enable\n");
    myHandler->enable();


    //Drive at a speed
    printf("Drive 10m/s turn\n");
    myHandler->setSpeed(10, 20);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //Drive at a speed
    printf("Drive 30m/s\n");
    myHandler->setSpeed(30, 30);
    std::this_thread::sleep_for(std::chrono::seconds(3));


    //Now try in torque mode
    printf("#### Change Mode to Torque\n");
    myHandler->setMode(TorqueDriveMode::UNLOCKED_TORQUE);

    //Drive at a speed
    printf("Drive 10m/s turn\n");
    myHandler->setSpeed(10, 20);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //Drive at a speed
    printf("Drive 30m/s\n");
    myHandler->setSpeed(30, 30);
    std::this_thread::sleep_for(std::chrono::seconds(3));


    printf("Clean Up\n");
    delete myHandler;
    printf("Done\n");
}