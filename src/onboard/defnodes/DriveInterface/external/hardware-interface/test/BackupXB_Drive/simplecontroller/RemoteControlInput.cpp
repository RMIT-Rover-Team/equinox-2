#include <iostream>
#include "SimpleController.h"
#include <stdbool.h>
#include "TCPstreams.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

//For delay
#include <chrono>
#include <thread>  

int main(int argc, char** argv) {
    if (argc < 3){
        std::cout << "Usage: " << argv[0] << " IP_Address Port" << std::endl;
        return 0;
    }

    std::cout << "Connecting to " << argv[1] << " on port " << argv[2] << std::endl;

    //Decode the port as int
    int port = atoi(argv[2]);

    //Connect to the receiver
    int sock = openclient(argv[1], port);

    //Start the controller
    SimpleController myController(32, 32); // Sane Defaults

    bool updatedFlag = false;

    char buffer[(32 * sizeof(bool)) + (32 * sizeof(int))];
    std::cout << "Buffer Size: " << sizeof(buffer) << std::endl;

    while (true){
        myController.tick();

        //Scan to see if data is available
        for (int index = 0; index < 32; index++){
            if (myController.buttonAvailable(index) || myController.axisAvailable(index)){
                updatedFlag = true;
                myController.getButton(index);
                myController.getAxis(index);
            }
        }

        if (updatedFlag){
            std::cout << "Axis Update..." << std::endl;  
            
            //Copy the button states first into packet
            memcpy(buffer, myController.getButtonArray(), 32 * sizeof(bool));
            //Then the axis states
            memcpy(buffer + 32 * sizeof(bool), myController.getAxisArray(), 32 * sizeof(int));
            //Send the packet
            send(sock, buffer, sizeof(buffer), 0);
            //Clear the flag
            updatedFlag = false;
        }

        //Delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}