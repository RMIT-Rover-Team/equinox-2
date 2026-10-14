#include "Motors/odrive_wrapper.hpp"
#include "Motors/odrive_constant.hpp"
#include "Motors/odrive_control.hpp"
#include <iostream>


/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


ODriveWrapper::ODriveWrapper(ODriveController* mainController, int wheelID, int direction){
    this->myCont = mainController;
    this->wheelID = wheelID;
    this->direction = direction;

    this->torqueMode = 1; // Initially assume torque mode
    this->posOffset = 0;

    printf("Init Wheel %i direction %i\n", wheelID, direction);
}

void ODriveWrapper::calibrate(){
    this->myCont->clearErrors(this->wheelID);
    this->myCont->setAxisState(this->wheelID, ODriveAxisState::FULL_CALIBRATION_SEQUENCE); // Calibrate
    
    //Establish the pos offset
    this->myCont->requestEncoder(this->wheelID); // Damn you George with your async bullshit
    ODriveState state = this->myCont->getState(this->wheelID); // get the cached data
    this->posOffset = state.Pos_Estimate * this->direction;
}


void ODriveWrapper::enable(){
    if (torqueMode){
        this->myCont->setControlMode(this->wheelID, ODriveControlMode::TORQUE_CONTROL, ODriveInputMode::PASSTHROUGH); // Set Torque Mode
        printf("Wheel %i Set Mode Torque\n", this->wheelID);
    }
    else {
        this->myCont->setControlMode(this->wheelID, ODriveControlMode::VELOCITY_CONTROL, ODriveInputMode::VEL_RAMP); // Set Velocity Mode
        printf("Wheel %i Set Mode Velocity\n", this->wheelID);
    }
    this->myCont->setAxisState(this->wheelID, ODriveAxisState::CLOSED_LOOP_CONTROL); // Enable PID and motor movement

    this->myCont->setAccelLim(this->wheelID, 100.0);
}

void ODriveWrapper::disable(){
    this->myCont->setAxisState(this->wheelID, ODriveAxisState::IDLE);
}


void ODriveWrapper::setTorque(float value){
    //If we are not in torque mode, we need to disable and switch
    if (torqueMode != 1){
        torqueMode = 1;
        this->disable();
        this->enable();
    }
    this->myCont->setTorque(this->wheelID, value * this->direction);
}

void ODriveWrapper::setSpeed(float value) {
    //If we are not in speed mode, we need to disable and switch
    if (torqueMode != 0){
        torqueMode = 0;
        this->disable();
        this->enable();
    }
    this->myCont->setVelocity(this->wheelID, value * this->direction, 10.0); // Magic number mb XD
}

void ODriveWrapper::clearErrors(){
    this->myCont->clearErrors(this->wheelID);
}

void ODriveWrapper::estop(){
    this->myCont->estop(this->wheelID);
}

float ODriveWrapper::getSpeed(){
    this->myCont->requestEncoder(this->wheelID); // Damn you George with your async bullshit
    ODriveState state = this->myCont->getState(this->wheelID); // get the cached data
    return state.Vel_Estimate * this->direction;
}


float ODriveWrapper::getPos(){
    this->myCont->requestEncoder(this->wheelID); // Damn you George with your async bullshit
    ODriveState state = this->myCont->getState(this->wheelID); // get the cached data
    return (state.Pos_Estimate * this->direction) - this->posOffset;
}
