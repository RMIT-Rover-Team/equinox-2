#pragma once

#include "GenericMotor.h"
#include "GenericCan.h"

#define SingleMotorMsgIDOffset 0x140
#define SingleMotorReplyIDOffset 0x240
#define MaxTorque 255
#define SpeedLimit 100

enum DriveMode {
    VelocityMode,
    PositionMode
};

class MyActuatorMotor: public GenericMotor {
  public:
    MyActuatorMotor(int targetID, GenericCan* can);

    void calibrate();
    double getPosition();
    void setPosition(double pos);
    void estop();
    void stop();
    void tick();
    void setVelocity(double vel);

    ~MyActuatorMotor();

  private:
    int myID;
    GenericCan* myCan;

};