//C++ TCPstreams Version 1.0

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string> 
#include <cstring>
#include <arpa/inet.h>
#include "TCPstreams.h"

//The Address Struct
struct sockaddr_in address;

int openserver(std::string IP, int port){
    int opt = 1;
    int server;
    


    if (IP == "0.0.0.0"){
        address.sin_addr.s_addr = INADDR_ANY;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons( port );


    

    //Open Socket
    server = socket(AF_INET, SOCK_STREAM, 0);
    
    //If Socket Open Failure
    if ( (server == 0) || (server == -1) ) {
        return -1; //Error 1
    }

    //Connect to Socket
    if ( setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))  ) {
        return -2; //Error 2
    }

    //bind to port
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0){
        return -3; //Error 3
    }

    //Now Listen on that port
    if (listen(server, 100) < 0){
        return -4; //Error 4
    }

    return server;



}


int taccept(int server){
    int new_socket;
    int sizeaddr = sizeof(address);
    new_socket = accept(server, (struct sockaddr *)&address, (socklen_t*)&sizeaddr);
    if (new_socket < 0){
        return -5; //Error 5
    }
    return new_socket;
}

void tsenddat(int socketin, char* MSG){
    send(socketin, MSG, strlen(MSG), 0);

}

std::string tgetdat(int socketin, int buffer){
    char indat[buffer+1];
    std::string outstring;

    //Wipe buffer
    for (int i = 0; i < buffer+1; i++){
        indat[i] = 0;
    }

    //Now Read
    read(socketin, indat, buffer);

    //Write to Output string
    //for (int i = 0; i < buffer; i++){
    //    outstring.append(indat[i]);
    //}
    outstring.append(indat);

    return outstring;


}


int openclient(std::string IP, int port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0){
        return -1; //Error 1
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    //Convert IP Addresses
    if ( inet_pton(AF_INET, IP.c_str(), &address.sin_addr) <= 0 ){
        return -2; // Error 2
    }
    //Connect to Socket
    if ( connect(sock, (struct sockaddr *)&address, sizeof(address) ) < 0  ){
        return -3; //Error 3
    }

    return sock;

}
