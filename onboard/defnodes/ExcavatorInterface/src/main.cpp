#include <iostream>

//Include your channel type(s)
#include <ChannelLossycast.hpp>
#include <ExcavatorNode.h>
#include <universalcan/RoverCanMaster.h>
#include <universalcan/SocketCAN.h>



int main(int argc, char *argv[]) {
    // Check if Canbus interface is provided as argument
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <canbus_interface>" << std::endl;
        return 1;
    }

    // Establish a CANBus
    SocketCAN can_bus(argv[1]);
    std::cout << "Attached to CAN Bus" << std::endl;

    // Start the Can Master
    RoverCanMaster can_master(can_bus);
    std::cout << "Started CAN Master" << std::endl;


    ExcavatorNode node(can_master);
    node.run();

    return 0;
}
