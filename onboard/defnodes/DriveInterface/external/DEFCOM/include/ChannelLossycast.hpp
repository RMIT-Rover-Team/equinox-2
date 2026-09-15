#pragma once
#include "comms/Multicast.hpp"
#include "DefComParser.hpp"
#include "FlexibleMessageStructure.hpp"
#include <string>

class LossyCastPublisher{
    private:
        ConnectionSpecification definition;
        int sequence = 63;
        udpsend* sock;

    public:
        LossyCastPublisher(std::string filePath);
        ~LossyCastPublisher();

        MessageStructure getNewMessageObject();

        void publish(MessageStructure requestData);
};

class LossyCastSubscriber{
    private:
        ConnectionSpecification definition;
        udpget* sock;
        int lastSequence = 0;

    public:
        LossyCastSubscriber(std::string filePath);
        ~LossyCastSubscriber();

        MessageStructure subscribe();

};