#include <iostream>
#include "DefComParser.hpp"
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
    cout << "Testing Message Structure" << endl;

    //Construct the input dict
    std::map<std::string, std::string> inputDict = {
        {"myVarA", "int"},
        {"myVarB", "float"},
        {"myVarC", "double[3]"},
        {"myVarD", "long"},
        {"myVarE", "char[5]"},
        {"myVarF", "string[16]"},
        {"myVarG", "bytes[4]"}
    };

    std::vector<std::string> order = {"myVarA", "myVarB", "myVarC", "myVarD", "myVarE", "myVarF", "myVarG"};
    

    //Construct the message structure
    cout << "Make Structure" << endl;
    MessageStructure ms(inputDict, order);
    cout << "Structure size is: " << ms.totalSize << endl;

    //Set the values
    ms.setInt("myVarA", 10);
    cout << "int is: " << ms.getInt("myVarA") << " should be 10" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    ms.setFloat("myVarB", 123.45);
    cout << "float is: " << ms.getFloat("myVarB") << " should be 123.45" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    ms.setDouble("myVarC", -123.456789,2);
    cout << "double is: " << ms.getDouble("myVarC",2) << " should be -123.456789" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    ms.setLong("myVarD", 1234567890);
    cout << "long is: " << ms.getLong("myVarD") << " should be 1234567890 (or 499602d2 if hex)" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    ms.setChar("myVarE", 'a');
    cout << "char is: " << ms.getChar("myVarE") << " should be a" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    ms.setString("myVarF", "Hello World!");
    cout << "string is: " << ms.getString("myVarF") << " should be Hello World!" << endl;
    hexDump(ms.getDecodeBuffer(), ms.totalSize);

    uint8_t bbytes[4] = {0x01,0x02,0x03,0x04};
    ms.setBytes("myVarG", bbytes);
    memset(bbytes, 0, 4);
    ms.getBytes("myVarG", bbytes);
    cout << "bytes is: ";
    hexDump(bbytes, 4);
    hexDump(ms.getDecodeBuffer(), ms.totalSize);


    return 0;
}