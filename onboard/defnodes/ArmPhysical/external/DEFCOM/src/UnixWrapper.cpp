#include "comms/GenericNetWrapper.hpp"
#include "comms/UnixWrapper.hpp"

#include <thread>
#include <chrono>
#include <signal.h>

int newUnixServer(const std::string& path) {
    //Create the sokcet
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un serv{};
    serv.sun_family = AF_UNIX;
    strncpy(serv.sun_path, path.c_str(), sizeof(serv.sun_path)-1);

    //Remove existing socket if it is there
    unlink(path.c_str());
    //Bind to the file
    bind(server_fd, (sockaddr*)&serv, sizeof(serv));
    listen(server_fd, 1);

    //Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    return server_fd;
}


UnixClientCon::UnixClientCon(const std::string& path) {
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un serv{};
    serv.sun_family = AF_UNIX;
    std::strncpy(serv.sun_path, path.c_str(), sizeof(serv.sun_path) - 1);

    //Await a connection to be available
    int constatus;
    while (true){
        constatus = connect(sockfd, (sockaddr*)&serv, sizeof(serv));

        if (constatus == 0){
            break;
        }
        //Delay 1 second
        std::cout << "Connection Refused to " << path << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    //Set up the statistics
    double now = time(nullptr);
    info.InitTime = now;
    info.LastPacket = now;
}


UnixClientCon::~UnixClientCon() {
    this->closeCon();
}


UnixServerCon::UnixServerCon(int server_fd) {
    // Accept the connection and save the address
    struct sockaddr_un addr{};
    socklen_t sizeaddr = sizeof(addr);

    this->sockfd = accept(server_fd, (struct sockaddr*)&addr, &sizeaddr);

    // Set up the statistics
    double now = time(nullptr);
    this->info.InitTime = now;
    this->info.LastPacket = now;
}

UnixServerCon::~UnixServerCon() {
    this->closeCon();
}