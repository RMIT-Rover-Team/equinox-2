#include <iostream>
#include <map>
#include <iomanip>
#include "DefComParser.hpp"

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
    cout << "Testing Parser" << endl;

    if (argc != 2){
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    cout << "Test Name2Port " << nameToPort("Test") << endl;
    cout << "Test FQDN DN" << resolveFQDN("localhost") << endl;
    cout << "Test FQDN IP" << resolveFQDN("127.0.0.1") << endl;

    cout << "Test Strip Whitespace " << stripWhitespace("the quick brown fox") << endl;

    cout << endl << "test loading a file: " << endl;
    ConnectionSpecification myCon = loadConfFile(argv[1]);

    //Display the name
    cout << "\t- Connection Name: " << myCon.Name << endl;
    cout << "\t- Resolved IP: " << myCon.ResolvedIP << endl;
    cout << "\t- Port: " << myCon.NumericPort << endl;

    //Display the request message format
    cout << "\t- Request Message Format: " << endl;
    myCon.RequestMessageFormat.dump();
    cout << "\t- Response Message Format: " << endl;
    myCon.ResponseMessageFormat.dump();
    
    return 0;
}