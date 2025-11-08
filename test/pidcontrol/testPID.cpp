#include "pid_calc.hpp"
#include "linear_calc.hpp"
#include <iostream>

//This function simulates the feedback
float simFeedback(float input){
    return input * 20;
}

int main(){
    PID_Calc myPIDCalc(0.01, 0.0050, 0.001, -1, 1);
    Linear_Calc myLinearCalc(0.1, -1, 1);

    double timeStep;
    double VelocityValuePID;
    double VelocityValueLinear;
    double TorqueValuePID;
    double TorqueValueLinear;
    
    TorqueValuePID = 0;
    TorqueValueLinear = 0;

    myPIDCalc.set(10);
    myLinearCalc.set(10);

    for (int i = 0; i < 120; i++){
        VelocityValueLinear = simFeedback(TorqueValueLinear);
        VelocityValuePID = simFeedback(TorqueValuePID);

        TorqueValueLinear = myLinearCalc.update(VelocityValueLinear, 1);
        TorqueValuePID = myPIDCalc.update(VelocityValuePID, 1);

        printf("Linear Iter %i, step %f value %f torque %f\n",  i, timeStep, VelocityValueLinear, TorqueValueLinear);
        printf("PID    Iter %i, step %f value %f torque %f\n\n",  i, timeStep, VelocityValuePID, TorqueValuePID);
    }

    myPIDCalc.set(5);
    myLinearCalc.set(5);

    for (int i = 0; i < 120; i++){
        VelocityValueLinear = simFeedback(TorqueValueLinear);
        VelocityValuePID = simFeedback(TorqueValuePID);

        TorqueValueLinear = myLinearCalc.update(VelocityValueLinear, 1);
        TorqueValuePID = myPIDCalc.update(VelocityValuePID, 1);

        printf("Linear Iter %i, step %f value %f torque %f\n",  i, timeStep, VelocityValueLinear, TorqueValueLinear);
        printf("PID    Iter %i, step %f value %f torque %f\n\n",  i, timeStep, VelocityValuePID, TorqueValuePID);
    }
}