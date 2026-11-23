#include "ChannelLossycast.hpp"
#include "comms/Multicast.hpp"
#include "DefComParser.hpp"
#include "FlexibleMessageStructure.hpp"
#include <string>
#include <stdint.h>

LossyCastPublisher::LossyCastPublisher(std::string filePath){
    this->definition = loadConfFile(filePath);

    if (this->definition.RequestMessageFormat.totalSize > 1000){
        throw std::runtime_error("Message size too large for UDP");
    }

    sock = new udpsend(this->definition.ResolvedIP, this->definition.NumericPort, this->definition.RequestMessageFormat.totalSize + 8);
    sequence = 63;


    // Assign the priority
    sock->setPriority(this->definition.Priority);
}

LossyCastPublisher::~LossyCastPublisher(){
    delete sock;
}

MessageStructure LossyCastPublisher::getNewMessageObject(){
    return MessageStructure(this->definition.RequestMessageFormat);
}

void LossyCastPublisher::publish(MessageStructure requestData){
    unsigned char outgoing[this->definition.RequestMessageFormat.totalSize + 8];
    uint64_t snum = sequence;
    memcpy(outgoing, &snum, 8);
    memcpy(outgoing + 8, requestData.getDecodeBuffer(), this->definition.RequestMessageFormat.totalSize);
    
    sequence++;
    sock->senddat(outgoing);
}


LossyCastSubscriber::LossyCastSubscriber(std::string filePath){
    this->definition = loadConfFile(filePath);

    if (this->definition.RequestMessageFormat.totalSize > 1000){
        throw std::runtime_error("Message size too large for UDP");
    }

    sock = new udpget(this->definition.ResolvedIP, this->definition.NumericPort, this->definition.RequestMessageFormat.totalSize + 8);
    lastSequence = 0;
}

LossyCastSubscriber::~LossyCastSubscriber(){
    delete sock;
}

MessageStructure LossyCastSubscriber::subscribe(){
    unsigned char* incoming;
    while (true){
        incoming = sock->getdat();

        uint64_t sequence;
        memcpy(&sequence, incoming, 8);

        if (((sequence - this->lastSequence) % 64) < 32) {
            lastSequence = sequence;
            break;
        }

    }

    MessageStructure output = MessageStructure(this->definition.RequestMessageFormat);
    output.setDecodeBuffer(incoming + 8);
    
    return output;


}