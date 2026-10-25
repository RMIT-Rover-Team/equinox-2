#pragma once
#include <string>
#include "FlexibleMessageStructure.hpp"
#include "DefComParser.hpp"
#include "comms/GenericNetWrapper.hpp"
#include <vector>
#include <thread>
#include <mutex>

class TXServerChannel {
    public:
        TXServerChannel(std::string defComFile, void (*handlerHook)(MessageStructure* request, MessageStructure* response));
        ~TXServerChannel();

        void start();

    private:
        ConnectionSpecification definition;

        //The hook
        void (*handlerHook)(MessageStructure* request, MessageStructure* response);

        //Hook mutex
        std::mutex hookMutex;

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
        void clientProcess(GenericNetWrapper* client);

};

class TXClientChannel {
    public:
        TXClientChannel(std::string defComFile);
        ~TXClientChannel();

        //Message Creator
        MessageStructure getNewRequestObject();

        //Request
        MessageStructure request(MessageStructure requestData);

    private:
        ConnectionSpecification definition;
        GenericNetWrapper* con;
};