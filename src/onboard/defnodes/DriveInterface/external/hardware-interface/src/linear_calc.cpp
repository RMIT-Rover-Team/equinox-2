#include "MathModels/linear_calc.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


Linear_Calc::Linear_Calc(float moveRate, float min, float max){
    this->min = min;
    this->max = max;

    this->moveRate = moveRate;
    this->setpoint = 0;
    this->lastValue = 0;
}


void Linear_Calc::set(float setpoint){
    this->setpoint = setpoint;
}

float Linear_Calc::update(float measured, float timestep){
    float output = lastValue;

    if (measured > setpoint){
        output -= this->moveRate * timestep;
    }
    if (measured < setpoint){
        output += this->moveRate * timestep;
    }

    if (output > max){
        output = max;
    }
    if (output < min){
        output = min;
    }
    this->lastValue = output;

    return output;
}