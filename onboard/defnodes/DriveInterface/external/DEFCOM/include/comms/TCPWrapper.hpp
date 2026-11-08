#pragma once
#include "comms/GenericNetWrapper.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int newTCPServer(const std::string& Host, int Port);

class TCPClientCon : public GenericNetWrapper {
    public:
        TCPClientCon(const std::string& Host, int Port);
        ~TCPClientCon();
};

class TCPServerCon : public GenericNetWrapper {
    public:
        TCPServerCon(int server_fd);
        ~TCPServerCon();

    private:
        struct sockaddr_in address;
};