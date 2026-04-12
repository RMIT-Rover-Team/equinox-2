#pragma once

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


/* Call order:
Calibrate -> Enable -> All others -> Clear Errors -> All Others 
System must recalibrate after estop
*/
class DriveMotorWrapper {
    public:
        virtual void calibrate()  = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
        virtual void setTorque(float value)  = 0;
        virtual void setSpeed(float value)  = 0;
        virtual void clearErrors() = 0;
        virtual void estop() = 0;
        virtual float getSpeed() = 0;
        virtual float getPos() = 0;
};