#include "pid_calc.hpp"

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


PID_Calc::PID_Calc(float kp, float ki, float kd, float min, float max){
    this->kp = kp;
    this->kd = kd;
    this->ki = ki;
    
    this->min = min;
    this->max = max;

    this->previous_error = 0;
    this->integral = 0;

    this->setpoint = 0;

    this->lastValue = 0;
}


void PID_Calc::set(float setpoint){
    this->setpoint = setpoint;
}

float PID_Calc::update(float measured, float timestep){
    //Calculate the error
    float error = this->setpoint - measured;

    //Update the calculus values
    this->integral += error * timestep;
    float derivative = (error - this->previous_error) / timestep;
    this->previous_error = error;

    //Calculate the next PID value
    double PIDresult = (this->kp * error) + (this->ki * this->integral) + (this->kd * derivative);

    //Apply the PID value
    double output = PIDresult;

    if (output > max){
        output = max;
    }
    if (output < min){
        output = min;
    }

    //this->lastValue += (output * timestep);

    return output;
}