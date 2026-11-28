#include <iostream>
#include "ChannelTransactional.hpp"
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

void handleMessage(MessageStructure* incoming, MessageStructure* outgoing){
    //Dump the incoming message
    cout << "Received " << incoming->getInt("One") << " " << incoming->getFloat("Two") << endl;

    //Set a the outgoing message
    outgoing->setString("Three", "Hello!");
}

int main(int argc, char** argv) {
    cout << "Testing Transactional Client" << endl;

    TXClientChannel myClient("basicTest.defcom");

    int i = 0;
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1)); 

        MessageStructure outgoing = myClient.getNewRequestObject();
        outgoing.setInt("One", i);
        outgoing.setFloat("Two", i/3.0);

        MessageStructure incoming = myClient.request(outgoing);
        cout << "Received " << incoming.getString("Three") << endl;

        i += 1;

    }

    
    return 0;
}