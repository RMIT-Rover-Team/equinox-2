#include <iostream>
#include "ChannelMulticast.hpp"
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

int main(int argc, char** argv) {
    cout << "Testing Multicast Subscriber" << endl;

    MulticastSubscriber myCast("basicTest.defcom");

    while (true){
        MessageStructure myMessage = myCast.subscribe();
        std::cout << "Received " << myMessage.getInt("One") << " " << myMessage.getFloat("Two") << std::endl;
    }
    return 0;
}