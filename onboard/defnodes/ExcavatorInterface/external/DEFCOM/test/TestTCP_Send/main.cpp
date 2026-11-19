#include <iostream>
#include "comms/TCPWrapper.hpp"
#include <map>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

void hexDump(uint8_t* buffer, int size){
    for(int i = 0; i < size; i++){
        cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
        if (i % 16 == 15){
            cout << endl;
        }
    }
    cout << endl;
}

int main() {
    cout << "Testing TCP Sender" << endl;

    //Create a server
    int myServer = newTCPServer("127.0.0.1", 5000);
    cout << "Awaiting Connection..." << endl;
    TCPServerCon myCom(myServer);

    while (true) {
        cout << "Sending..." << endl;
        myCom.senddat((unsigned char*)"123456789", 10);
        //Delay 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
        
    }

    return 0;
}