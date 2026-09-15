#include "comms/GenericNetWrapper.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


bool GenericNetWrapper::senddat(unsigned char* data, int bufsize) {
        ssize_t sent = send(sockfd, data, bufsize, 0);
        if (sent <= 0) { 
            this->closeCon(); 
            return false; 
        }
        
        //Update the statistics
        info.TotalSent += sent;
        return true;
    }

unsigned char* GenericNetWrapper::getdat(int bufsize) {
    //If the buffer doesnt exist, create it
    if (buffer == nullptr) {
        buffer = new unsigned char[bufsize]; // Look Jonathan, I'm not using malloc this time!!!
    }

    memset(buffer, 0, bufsize);

    ssize_t rec = recv(sockfd, buffer, bufsize, 0);
    if (rec <= 0) { 
        this->closeCon(); 
        return nullptr; 
    }

    //Update the statistics
    info.TotalRecv += rec;
    info.LastPacket = time(nullptr);
    return buffer;
}

void GenericNetWrapper::closeCon() {
    if (info.Alive) {
        close(sockfd);
        info.Alive = false;
    }

    //If the buffer still exists, delete it
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
}

ConnInfo GenericNetWrapper::report(){
    return info;
}
