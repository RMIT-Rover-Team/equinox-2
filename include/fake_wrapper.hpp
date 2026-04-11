#pragma once
#include "motor_wrapper.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


class FakeWrapper: public  DriveMotorWrapper {
    private:
        int wheelID;
        float speed;
        float pos;

    public:
        //We need to share the controller with other instances
        FakeWrapper(int wheelID);


        void enable() override;
        void disable() override;
        void calibrate() override;
        void setTorque(float value)  override;
        void setSpeed(float value)  override;
        void clearErrors() override;
        void estop() override;
        float getSpeed() override;
        float getPos() override;
};