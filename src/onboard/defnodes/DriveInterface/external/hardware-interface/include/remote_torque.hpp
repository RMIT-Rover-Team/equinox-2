#pragma once
#include "torque_handler.hpp"

class RemoteHandler {
    private:
        int myClient;
    public:
        RemoteHandler(const char* ip, int port);

        void setSpeed(float leftSpeed, float rightSpeed);
        void calibrate();
        void enable();
        void disable();
        void estop();

        void setMode(TorqueDriveMode mode);

        OdomReading getOdom();

        ~RemoteHandler();
};