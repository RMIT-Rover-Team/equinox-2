#include "remote_torque.hpp"

#include "torque_handler.hpp"
#include "TCPstreams.h"
#include <sys/socket.h> 

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


RemoteHandler::RemoteHandler(const char* ip, int port){
    myClient = openclient(ip, port);
}

void RemoteHandler::setSpeed(float leftSpeed, float rightSpeed){
    union convBuf {
        char buffer[16];
        double leftRight[2];
    } myBuf;

    myBuf.leftRight[0] = leftSpeed;
    myBuf.leftRight[1] = rightSpeed;

    printf("Send set %lf %lf\n",leftSpeed, rightSpeed);

    send(myClient, myBuf.buffer, 16, 0);

}



void RemoteHandler::calibrate(){
    
}

void RemoteHandler::enable(){

}

void RemoteHandler::disable(){

}

void RemoteHandler::estop(){

}

void RemoteHandler::setMode(TorqueDriveMode mode){

}



OdomReading RemoteHandler::getOdom()
{
    OdomReading output;
    output.leftPos = 0;
    output.rightPos = 0;
    output.leftSpeed = 0;
    output.rightSpeed = 0;

    return output;
}

RemoteHandler::~RemoteHandler(){
    
}


