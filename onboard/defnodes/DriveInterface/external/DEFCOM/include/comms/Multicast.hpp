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

/*const int priMap[8] = {
    0x00, // PCP 0 - Best Effort
    0x20, // PCP 1
    0x40, // PCP 2
    0x60, // PCP 3
    0x80, // PCP 4
    0xA0, // PCP 5 - Critical
    0xC0, // PCP 6 - Internetwork Control
    0xE0  // PCP 7 - Network Control
};*/

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
