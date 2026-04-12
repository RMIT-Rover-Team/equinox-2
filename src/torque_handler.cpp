#include "torque_handler.hpp"
#include <thread>
#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/
TorqueHandler::TorqueHandler(const char* canInterface){
    //Initialise the communication handlers
    #ifndef TorqueHandler_TestMode
        #ifdef USE_ODrive
            this->myController = new ODriveController(canInterface);
        #endif
        #ifdef USE_MyActuator
            this->myCan = new WrappedCANBus(canInterface);
        #endif
    #endif

    //Set up all the motors
    for (int i = 0; i < MotorCount; i++){
        printf("Init motor %i\n", MotorIDs[i]);

        //Handle the test case - if we are in test mode we create a virtual motor, otherwise we create an actual odrive
        #ifdef TorqueHandler_TestMode
            this->myMotors[i] = new FakeWrapper(MotorIDs[i]);
        #else
            #ifdef USE_ODrive
                this->myMotors[i] = new ODriveWrapper(this->myController, MotorIDs[i], MotorDirs[i]);
            #endif
            #ifdef USE_MyActuator
                this->myMotors[i] = new MyActuatorMotor(this->myCan, MotorIDs[i], MotorDirs[i]);
            #endif
        #endif

        printf("Init motor %i %i %i\n",i, MotorIDs[i], MotorDirs[i]);
        
    }

    //Default config
    this->actionFlag = MotorAction::CONTROL_LOOP;
    this->mode = TorqueDriveMode::UNLOCKED_VELOCITY;
    
    printf("Ready\n");

    //Start the control thread
    this->loopControlFlag = 1;
    controlThread = std::thread(&TorqueHandler::controlLoop, this);

}

void TorqueHandler::setSpeed(float leftSpeed, float rightSpeed){
    this->leftSpeed = leftSpeed;
    this->rightSpeed = rightSpeed;
}



void TorqueHandler::calibrate(){
    actionFlag = MotorAction::CALIBRATE;
}

void TorqueHandler::enable(){
    actionFlag = MotorAction::ENABLE;
}

void TorqueHandler::disable(){
    actionFlag = MotorAction::DISABLE;
}

void TorqueHandler::estop(){
    actionFlag = MotorAction::ESTOP;
}

void TorqueHandler::setMode(TorqueDriveMode mode){
    this->mode = mode;
}


//This is the main motor control loop
void TorqueHandler::controlLoop(){
    printf("Motor Control Async Thread Begin\n");
    while (this->loopControlFlag){

        //Handle the action
        switch (this->actionFlag){
            case MotorAction::CALIBRATE:
                for (int i = 0; i < MotorCount; i++){
                    myMotors[i]->calibrate();
                }
                this->actionFlag = MotorAction::CONTROL_LOOP;
                break;
            case MotorAction::ESTOP:
                for (int i = 0; i < MotorCount; i++){
                    myMotors[i]->estop();
                }
                this->actionFlag = MotorAction::CONTROL_LOOP;
                break;
            case MotorAction::ENABLE:
                for (int i = 0; i < MotorCount; i++){
                    myMotors[i]->clearErrors();
                    myMotors[i]->enable();
                }
                this->actionFlag = MotorAction::CONTROL_LOOP;
                break;
            case MotorAction::DISABLE:
                for (int i = 0; i < MotorCount; i++){
                    myMotors[i]->disable();
                }
                this->actionFlag = MotorAction::CONTROL_LOOP;
                break;
            case MotorAction::CONTROL_LOOP:
                this->handlePID();
                break;
        };


        //Delay
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(ControlLoopTimestep*1000)));
    }
    printf("Motor Control Async Thread Terminate\n");
}

//This function is called by the main control loop and handles the torque distribution between wheels
void TorqueHandler::handlePID(){
    //printf("PID loop\n");
    switch (this->mode){
        //Unlocked Velocity mode just uses the inbuild velocity loop on the ODrives
        case TorqueDriveMode::UNLOCKED_VELOCITY:
            //Set the speed on the left and right wheels
            {
            int leftMotorIndex, rightMotorIndex;
            for (int i = 0; i < MotorsPerSide; i++){
                leftMotorIndex = LeftMotorIndexes[i];
                rightMotorIndex = RightMotorIndexes[i];

                myMotors[leftMotorIndex]->setSpeed(this->leftSpeed);
                myMotors[rightMotorIndex]->setSpeed(this->rightSpeed);
            }
            }
            break;
        

        case LOCKED_VELOCITY:
            //Fixed Factor for scaling
            double fixedFactor = abs(this->rightSpeed);

            //If they are not moving, we don't do anything special
            if ((fixedFactor < 0.1) && (fixedFactor > -0.1)) {
                //Write to wheels
                for (int i = 0; i < MotorsPerSide; i++){
                    //Write back
                    myMotors[LeftMotorIndexes[i]]->setSpeed(this->leftSpeed);
                    myMotors[RightMotorIndexes[i]]->setSpeed(this->rightSpeed);
                }
            }
            else {

                //Calculate the comparative scale factor between sides of the rover, this allows us to scale the speeds
                // such that they can be directly compared even when turning
                double leftRightScaleFactor = abs(this->leftSpeed / fixedFactor);

                //Extract the sign of left and right speeds, we perform all maths in positive domain then reapply the sign
                int leftSign = (this->leftSpeed < 0) ? -1 : 1;
                int rightSign = (this->rightSpeed < 0) ? -1 : 1;

                //Sample all the individual wheel speeds
                float leftSpeedsScaled[MotorsPerSide];
                float rightSpeedsScaled[MotorsPerSide];

                //Scale all the right hand side motors such that their speeds are relative to lefthand side
                for (int i = 0; i < MotorsPerSide; i++){
                    //Read speeds and scale accordingly
                    leftSpeedsScaled[i] = abs(1 * myMotors[LeftMotorIndexes[i]]->getSpeed());
                    rightSpeedsScaled[i] = abs(leftRightScaleFactor * myMotors[RightMotorIndexes[i]]->getSpeed());

                    /*printf("Wheel Pair Initial: %f %f Scaled %f %f\n",
                        myMotors[LeftMotorIndexes[i]]->getSpeed(),
                        myMotors[RightMotorIndexes[i]]->getSpeed(),
                        leftSpeedsScaled[i],
                        rightSpeedsScaled[i]
                    );*/
                }


                //Find the slowest wheel
                float slowestSpeed = leftSpeedsScaled[0];
                for (int i = 0; i < MotorsPerSide; i++){
                    if (leftSpeedsScaled[i] < slowestSpeed){
                        slowestSpeed = leftSpeedsScaled[i];
                    }
                    if (rightSpeedsScaled[i] < slowestSpeed){
                        slowestSpeed = rightSpeedsScaled[i];
                    }
                }
                //printf("Slowest: %f\n",slowestSpeed);

                //Compute the wheel speed average
                float wheelAverage = 0;
                for (int i = 0; i < MotorsPerSide; i++){
                    wheelAverage += leftSpeedsScaled[i];
                    wheelAverage += rightSpeedsScaled[i];
                }
                wheelAverage = wheelAverage / (MotorsPerSide*2);

                //Average with target and slowest to produce weighted average that prefers the target speed
                float weightedAverage = (wheelAverage + slowestSpeed + abs(this->leftSpeed)) / 3.0;

                //printf("Computed average: %f\n",weightedAverage);

                //Scale back to left and right
                double leftNew, rightNew;
                leftNew = (weightedAverage / 1.0);
                rightNew = (weightedAverage / leftRightScaleFactor);

                //Enforce limits
                if (leftNew > abs(this->leftSpeed)){
                    leftNew = abs(this->leftSpeed);
                }

                if (rightNew > abs(this->rightSpeed)){
                    rightNew = abs(this->rightSpeed);
                }

                //Apply the sign
                leftNew = leftNew * leftSign;
                rightNew = rightNew * rightSign;
                
                //printf("New Speeds: %f %f Target: %f %f\n",leftNew, rightNew, this->leftSpeed, this->rightSpeed);

                //Write to wheels
                for (int i = 0; i < MotorsPerSide; i++){
                    //Write back
                    myMotors[LeftMotorIndexes[i]]->setSpeed(leftNew);
                    myMotors[RightMotorIndexes[i]]->setSpeed(rightNew);
                }
            }
            
            break;
    };
}


OdomReading TorqueHandler::getOdom()
{
    OdomReading output;
    output.leftPos = 0;
    output.rightPos = 0;
    output.leftSpeed = 0;
    output.rightSpeed = 0;

    //Sample the speeds
    for (int i = 0; i < MotorsPerSide; i++){
        //Write back
        output.leftPos += myMotors[LeftMotorIndexes[i]]->getPos();
        output.rightPos += myMotors[RightMotorIndexes[i]]->getPos();

        output.leftSpeed += myMotors[LeftMotorIndexes[i]]->getSpeed();
        output.rightSpeed += myMotors[RightMotorIndexes[i]]->getSpeed();
    }
    output.leftPos /= MotorsPerSide;
    output.rightPos /= MotorsPerSide;
    output.leftSpeed /= MotorsPerSide;
    output.rightSpeed /= MotorsPerSide;
    return output;
}

TorqueHandler::~TorqueHandler(){
    //Await thread kill
    this->loopControlFlag = 0;
    this->controlThread.join();

    for (int i = 0; i < MotorCount; i++){
        printf("Cleanup motor %i\n",i);
        delete this->myMotors[i];
    }

    //Clean up
    #ifndef TorqueHandler_TestMode
        #ifdef USE_ODrive
            delete this->myController;
        #endif
        #ifdef USE_MyActuator
            delete this->myCan;
        #endif
    #endif
}


