#include <iostream>
#include "comms/UnixWrapper.hpp"
#include <map>
#include <iomanip>

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
    cout << "Testing Unix Receiver" << endl;

    UnixClientCon myCom("/tmp/test");

    while (true) {
        unsigned char* buffer = myCom.getdat(10);
        hexDump(buffer, 10);
    }
    return 0;
}