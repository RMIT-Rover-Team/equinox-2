#include "Motors/drive_motor_wrapper.hpp"
#include "Motors/fake_wrapper.hpp"
#include <iostream>


int main(){
    DriveMotorWrapper* myWrap = new FakeWrapper(10);
    myWrap->calibrate();
    myWrap->enable();
    myWrap->setTorque(0.5);
    printf("Speed: %f\n", myWrap->getSpeed());
    myWrap->setSpeed(50);
    printf("Speed: %f\n", myWrap->getSpeed());
    myWrap->estop();
    myWrap->clearErrors();
    myWrap->disable();
    myWrap->enable();

    delete myWrap;
}