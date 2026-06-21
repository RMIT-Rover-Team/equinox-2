#include <iostream>
#include "../libuniversalcan/RoverCanSlave.h"
#include "../libuniversalcan/SocketCanWrapper.h"

int main(){
    WrappedCANBus myCan("vcan0");

    printf("Init Ratcan\n");

    RoverCanSlave mySlave(2, &myCan);
    printf("Init Slave\n");

    while (true) {
        printf("\nListening....\n");
        mySlave.listen();

        //printf("\nAwaiting next command....\n");
    }
}