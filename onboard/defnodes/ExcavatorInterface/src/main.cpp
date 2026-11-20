#include <iostream>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
#include <ExcavatorPayload.hpp>
#include <RoverCanMaster.hpp>
#include <SocketCanWrapper.h>
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>

int main(int argc, char *argv[]) {
    // Check if Canbus interface is provided as argument
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <canbus_interface>" << std::endl;
        return 1;
    }

    // Establish a CANBus
    WrappedCANBus myBus(argv[1]);
    std::cout << "Attached to CAN Bus" << std::endl;
    
    // Start the Can Master
    RoverCanMaster can_master(myBus);
    can_master.start_listen_thread();
    std::cout << "Started Master Lib" << std::endl;

    // Establish the Excavator Payload
    ExcavatorPayload excavator(can_master);
    std::cout << "Started Excavator Payload" << std::endl;

    std::cout << "Hello World" << std::endl;
    return 0;
}