#include "fake_wrapper.hpp"
#include <iostream>

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


FakeWrapper::FakeWrapper(int wheelID){
     printf("Init Fake Wheel %i\n", wheelID);
     this->wheelID = wheelID;
     this->speed = 0;
     this->pos = 0;
}


void FakeWrapper::enable(){
    printf("Wheel %i Enable\n", this->wheelID);
}

void FakeWrapper::disable(){
    printf("Wheel %i Disable\n", this->wheelID);
}

void FakeWrapper::calibrate(){
    printf("Wheel %i Calibrate\n", this->wheelID);
}

void FakeWrapper::setTorque(float value) {
    printf("Wheel %i SetTorque %f\n", this->wheelID, value);
    //make some shit up
    this->speed = 200*value;
}

void FakeWrapper::setSpeed(float value) {
    printf("Wheel %i SetVelocity %f\n", this->wheelID, value);
    this->speed = value;
     //Update the pos with the integral
    this->pos += value;
}

void FakeWrapper::clearErrors(){
    printf("Wheel %i Clear Error\n", this->wheelID);
}

void FakeWrapper::estop(){
    printf("Wheel %i ESTOP\n", this->wheelID);
}

float FakeWrapper::getSpeed(){
    printf("Wheel %i Getspeed %f\n", this->wheelID, this->speed);
    return this->speed;
}

float FakeWrapper::getPos(){
    printf("Wheel %i Getpos %f\n", this->wheelID, this->pos);
    return this->pos;
}