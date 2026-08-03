#include "MathModels/wheel_speed_cal.hpp"
#include <iostream>

int main() {
    WheelSpeedCalculator wheelSpeedCalculator(0.1, 0.05); // Example values for wheel radius and offset
    double v_linear = 1.0; // Linear velocity in m/s
    double v_angular = 0.5; // Angular velocity in rad/s
    printf("Wheel speeds: %lf %lf\n", 
           wheelSpeedCalculator.calculateLeftSpeed(v_linear, v_angular),
           wheelSpeedCalculator.calculateRightSpeed(v_linear, v_angular));
}