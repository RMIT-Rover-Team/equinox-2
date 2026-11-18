#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>



class udpsend {
public:
    udpsend(const std::string& ip, int port, int buff_size = 1024, std::string MCAST_GRP = "239.0.0.1");

    // Priorities 0 to 7
    void setPriority(int priority);

    void senddat(unsigned char* data);

    ~udpsend();

private:
    int sock;
    int buff_size;
    sockaddr_in addr{};
    std::string MCAST_GRP;
};


class udpget {
public:
    udpget(const std::string& ip, int port, int buff_size = 1024, std::string MCAST_GRP = "239.0.0.1");
    
    unsigned char* getdat();
    ~udpget();

    //Deprecated
    void join_multicast_all_interfaces(const std::string& group_ip);

private:
    int sock;
    int buff_size;
    unsigned char* buffer;
    std::string group;
};
