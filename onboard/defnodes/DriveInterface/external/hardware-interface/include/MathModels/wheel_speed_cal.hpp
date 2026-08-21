/*
    RMIT Rover Equinox
    Author: Myie
    Date: 02/07/2025
    Description: Wheel speed calculator for Equinox differential drive header file
*/
#ifndef WHEEL_SPEED_CALCULATOR_H
#define WHEEL_SPEED_CALCULATOR_H

class WheelSpeedCalculator {
public:
    WheelSpeedCalculator(double wheel_r, double x_offset);

    // Calculates rotational speed (rad/s)
    double calculateLeftSpeed(double v_linear, double v_angular);
    double calculateRightSpeed(double v_linear, double v_angular);

private:
    double r_wheel; // wheel rad (m)
    double x_offset; //wheel offset (m)

};

#endif // WHEEL_SPEED_CALCULATOR_H