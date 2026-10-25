#include "comms/GenericNetWrapper.hpp"
#include "comms/TCPWrapper.hpp"

#include <thread>
#include <chrono>
#include <signal.h>


int newTCPServer(const std::string& Host, int Port) {
    //Create the socket 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //Allow reuse of existing ports (in case they are in sock await)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    //Bind to the socket
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(Port);
    inet_pton(AF_INET, Host.c_str(), &serv.sin_addr);

    bind(server_fd, (sockaddr*)&serv, sizeof(serv));
    listen(server_fd, 1);

    //Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    
    return server_fd;
}


TCPClientCon::TCPClientCon(const std::string& Host, int Port) {
    //Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Bind to the socket
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(Port);
    inet_pton(AF_INET, Host.c_str(), &serv.sin_addr);

    int constatus;
    while (true){
        constatus = connect(sockfd, (sockaddr*)&serv, sizeof(serv));

        if (constatus == 0){
            break;
        }
        //Delay 1 second
        std::cout << "Connection Refused to " << Host << ":" << Port << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //Set up the statistics
    double now = time(nullptr);
    this->info.InitTime = now;
    this->info.LastPacket = now;
}

TCPClientCon::~TCPClientCon() {
    this->closeCon();
}



TCPServerCon::TCPServerCon(int server_fd) {
    //Accept the connection and save the address
    int sizeaddr = sizeof(struct sockaddr_in);
    this->sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&sizeaddr);

    //Set up the statistics
    double now = time(nullptr);
    this->info.InitTime = now;
    this->info.LastPacket = now;
}

TCPServerCon::~TCPServerCon() {
    this->closeCon();
}