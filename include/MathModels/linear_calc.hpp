#pragma once
#include "control_calc.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


class Linear_Calc: public Control_Calc {
    private:
        float setpoint; // the target value of the loop
        float moveRate; // The rate of increment/ decrement
        float lastValue; // The last value

        float min, max; // Min and max of system
    public:
        Linear_Calc(float moveRate, float min, float max);

        /*
        set.
        Sets the PID target value
        Inputs: The Setpoint value
        Returns: None
        */
        void set(float setpoint) override;

        /*
        update.
        Inputs: measured value, timestep (seconds)
        Outputs: New value for measured
        */
        float update(float measured, float timestep) override;
};