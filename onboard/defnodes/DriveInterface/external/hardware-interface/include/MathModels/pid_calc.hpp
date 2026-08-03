#pragma once
#include "control_calc.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


class PID_Calc: public Control_Calc {
    private:
        float kp; // Proporational Gain
        float ki; // Integral Gain
        float kd; // Derivatrive Gain

        float previous_error; // For Derivation
        float integral; // running sum for integration

        float setpoint; // the target value of the loop

        float min, max; // Min and max of system

        float lastValue;

    public:
        PID_Calc(float kp, float ki, float kd, float min, float max);

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