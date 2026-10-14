#include <iostream>
#include <stdbool.h>
#include "RemoteController.h"
#include "TCPstreams.h"
#include <sys/ioctl.h>
#include <cstring>
#include <unistd.h>


RemoteController::RemoteController(int port){
    memset(buttonStates, 0, sizeof(bool) * 32);
    memset(buttonReadStates, 0, sizeof(bool) * 32);

    memset(axisStates, 0, sizeof(int) * 32);
    memset(axisReadStates, 0, sizeof(bool) * 32);
    
    //Await TCP Connection
    std::cout << "Awaiting TCP Connection" << std::endl;
    int server = openserver("0.0.0.0", port);

    //Accept a connection
    this->client = accept(server);
    if (this->client < 0){
        std::cerr << "Error accepting connection" << std::endl;
    }

    std::cout << "Controller Ready" << std::endl;

}
RemoteController::~RemoteController(){

}

bool* RemoteController::getButtonArray(){
    return this->buttonStates;
}
int* RemoteController::getAxisArray(){
    return this->axisStates;
}

bool RemoteController::getButton(int b){
    this->buttonReadStates[b] = UNSET;
    return this->buttonStates[b];
}
int RemoteController::getAxis(int a){
    this->axisReadStates[a] = UNSET;
    return this->axisStates[a];
}

bool RemoteController::buttonAvailable(int b){
    return this->buttonReadStates[b];
}
bool RemoteController::axisAvailable(int a){
    return this->axisReadStates[a];
}

void RemoteController::tick(){
    //Get any events

    //Check for waiting bytes
    int waitingCount = 0;
    ioctl(this->client, FIONREAD, &waitingCount);

    if (waitingCount > 0){
        //Read the bytes
        char buffer[(32 * sizeof(bool)) + (32 * sizeof(int))];

        read(this->client, buffer, sizeof(buffer));

        //Back up old states
        memcpy(this->oldButtonStates, this->buttonStates, 32 * sizeof(bool));
        memcpy(this->oldAxisStates, this->axisStates, 32 * sizeof(int));

        //Unpack locally
        memcpy(this->buttonStates, buffer, 32 * sizeof(bool));
        memcpy(this->axisStates, buffer + 32 * sizeof(bool), 32 * sizeof(int));

        //Check for changes
        for (int index = 0; index < 32; index++){
            if (this->buttonStates[index] != this->oldButtonStates[index]){
                this->buttonReadStates[index] = AWAIT_READ;
            }
            if (this->axisStates[index] != this->oldAxisStates[index]){
                this->axisReadStates[index] = AWAIT_READ;
            }
        }
    }
    
}