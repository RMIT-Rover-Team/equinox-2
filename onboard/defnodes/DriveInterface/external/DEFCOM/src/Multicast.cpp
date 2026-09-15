#include "comms/Multicast.hpp"

udpsend::udpsend(const std::string& ip, int port, int buff_size) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0){
        throw std::runtime_error("Failed to create socket");
    }

    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buff_size, sizeof(buff_size));

    // TTL = 1
    unsigned char ttl = 1;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    // Save the buffer size
    this->buff_size = buff_size;
}

void udpsend::senddat(unsigned char* data) {
    sendto(sock, data, this->buff_size, 0, (sockaddr*)&addr, sizeof(addr));
}

udpsend::~udpsend() {
    close(sock);
}


udpget::udpget(const std::string& group_ip, int port, int buff_size) {
    //Create the receive buffer
    this->buffer = (unsigned char*)malloc(sizeof(unsigned char) * buff_size);

    group = group_ip;
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0){
        throw std::runtime_error("Failed to create socket");
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in bind_addr{};
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&bind_addr, sizeof(bind_addr)) < 0)
        throw std::runtime_error("Bind failed");

    join_multicast_all_interfaces(group_ip);
}

unsigned char* udpget::getdat() {
    ssize_t len = recv(sock, buffer, buff_size, 0);
    return buffer;
}

udpget::~udpget() {
    close(sock);
    free(this->buffer);
}



void udpget::join_multicast_all_interfaces(const std::string& group_ip) {
    struct ifaddrs* ifaddr;
    getifaddrs(&ifaddr);

    //Iterate over each interface
    for (auto* iface = ifaddr; iface != nullptr; iface = iface->ifa_next) {
        //Ignore non-IPv4 and therefor non-unicast addresses
        if (!iface->ifa_addr || iface->ifa_addr->sa_family != AF_INET){
            continue;
        }

        //Get the address
        auto* sa = (sockaddr_in*)iface->ifa_addr;
        ip_mreq mreq{};
        inet_pton(AF_INET, group_ip.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface = sa->sin_addr;

        //Set the socket options
        setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    }

    freeifaddrs(ifaddr);
}