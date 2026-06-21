#include <iostream>
#include "../libuniversalcan/RoverCanMaster.h"
#include "../libuniversalcan/SocketCanWrapper.h"
#include <tuple>

int main(){
    WrappedCANBus myCan("can0");
    printf("Init Ratcan\n");

    RoverCanMaster myMaster(myCan, MASTER_CAN_ID);
    printf("Init Master\n");

    while (true){
        std::pair<Datapoint, float> myDP = myMaster.BroadcastDataPoint();
        std::cout << myDP.first.from << " " << myDP.first.stream_id << " " << myDP.first.channel_id << " " << myDP.second << "\n";

    }
}