/*
    RMIT Rover RAT3
    Author: Myie
    Date: 02/07/2025
    Description: Wheel speed calculator for RAT3 differential drive
*/
#include "wheel_speed_cal.hpp"

WheelSpeedCalculator::WheelSpeedCalculator(double r_wheel, double x_offset){
    this -> r_wheel = r_wheel;
    this -> x_offset = x_offset;
}

double WheelSpeedCalculator::calculateLeftSpeed(double v_linear, double v_angular){
    // Calculate left wheel speed (rad/s)
    return (v_linear - v_angular * x_offset) / r_wheel;
}

double WheelSpeedCalculator::calculateRightSpeed(double v_linear, double v_angular){
    // Calculate right wheel speed (rad/s)
    return (v_linear + v_angular * x_offset) / r_wheel;
}