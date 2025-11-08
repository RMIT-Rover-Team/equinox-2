#pragma once
#include "odrive_control.hpp"
#include "motor_wrapper.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


class ODriveWrapper: public  MotorWrapper {
    private:
        ODriveController* myCont;
        int wheelID;
        int direction;

        int torqueMode;
        int posOffset;

    public:
        //We need to share the controller with other instances
        ODriveWrapper(ODriveController* mainController, int wheelID, int direction);
        
        void enable() override;
        void disable() override;
        void calibrate() override;
        void setTorque(float value)  override;
        void setSpeed(float value)  override;
        void clearErrors() override;
        void estop() override;
        float getSpeed() override;
        float getPos() override;
        int getWheelID() const { return wheelID; }
};
