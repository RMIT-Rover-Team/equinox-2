#pragma once

#include "drive_motor_wrapper.hpp"
#include "CanComms/GenericCan.h"

#define SingleMotorMsgIDOffset 0x140
#define SingleMotorReplyIDOffset 0x240
#define MaxTorque 255
#define SpeedLimit 100

enum DriveMode {
    VelocityMode,
    PositionMode
};

class MyActuatorMotor: public DriveMotorWrapper {
  public:
    MyActuatorMotor(int targetID, int dir, GenericCan* can);

    void calibrate() override;
    float getPos() override;
    void setPos(double pos);
    void estop() override;
    void stop();
    void tick();
    void setSpeed(float value) override;
    void getSpeed() override;

    

    //Dummy functions
    void enable() override;
    void disable() override;
    void setTorque(float value) override;
    void clearErrors() override;

    ~MyActuatorMotor();

  private:
    int myID;
    GenericCan* myCan;
    int dir;

};
