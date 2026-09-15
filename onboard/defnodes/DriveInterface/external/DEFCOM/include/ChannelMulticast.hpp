#pragma once
#include <string>
#include "FlexibleMessageStructure.hpp"
#include "DefComParser.hpp"
#include "comms/GenericNetWrapper.hpp"
#include <vector>
#include <map>
#include <thread>
#include <mutex>

class MulticastPublisher {
    public:
        MulticastPublisher(std::string defComFile);
        ~MulticastPublisher();

        MessageStructure getNewMessageObject();
        void publish(MessageStructure requestData);

    private:
        ConnectionSpecification definition;

        //Send Mutex
        std::mutex sendMutex;

        //The Send buffers
        std::map<int, std::vector<MessageStructure>> sendBuffers;

        //Client ID Counter
        int idCounter = 0;

        //The File Descriptors for both the TCP and Unix sockets
        int tcpServer;
        int unixServer;

        //TCP Acceptor thread
        std::thread acceptorThreadTCP;

        //Unix Acceptor thread
        std::thread acceptorThreadUnix;

        //Acceptor Mutex
        std::mutex acceptMutex;

        //Vector of client threads
        std::vector<std::thread> clientThreads;
        std::vector<GenericNetWrapper*> clientConnections;

        //The Acceptor loops for TCP and Unix
        void acceptorTCP();
        void acceptorUnix();

        //The client creator
        void spawnClient(GenericNetWrapper* client);

        //The client process
        void clientProcess(GenericNetWrapper* client, int clientID);

};

class MulticastSubscriber {
    public:
        MulticastSubscriber(std::string defComFile);
        ~MulticastSubscriber();

        //Request
        MessageStructure subscribe();

    private:
        ConnectionSpecification definition;
        GenericNetWrapper* con;
};