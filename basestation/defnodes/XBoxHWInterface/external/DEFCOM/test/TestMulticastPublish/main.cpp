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
    cout << "Testing MultiCast Publisher" << endl;

    MulticastPublisher myCast("basicTest.defcom");

    int counter = 0;
    double c2 = 0;
    while (true) {
        MessageStructure outgoing = myCast.getNewMessageObject();
        outgoing.setInt("One", counter);
        outgoing.setFloat("Two", c2);
        myCast.publish(outgoing);

        cout << "Sending " << counter << " " << c2 << endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(1)); 

        counter += 1;
        c2 = counter/2.0;
    }
    cout << "End" << endl;
    return 0;
}