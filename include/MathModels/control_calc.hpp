#pragma once

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/

//Parent Class for controllers
class Control_Calc {
    public:
        /*
        set.
        Sets the PID target value
        Inputs: The Setpoint value
        Returns: None
        */
        virtual void set(float setpoint) = 0;

        /*
        update.
        Inputs: measured value, timestep (seconds)
        Outputs: New value for measured
        */
        virtual float update(float measured, float timestep) = 0;
};