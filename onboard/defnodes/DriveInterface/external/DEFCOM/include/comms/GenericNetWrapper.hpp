#pragma once

const int priMap[8] = {
    0x00, // PCP 0 - Best Effort
    0x20, // PCP 1
    0x40, // PCP 2
    0x60, // PCP 3
    0x80, // PCP 4
    0xA0, // PCP 5 - Critical
    0xC0, // PCP 6 - Internetwork Control
    0xE0  // PCP 7 - Network Control
};

struct ConnInfo {
    long TotalSent = 0;
    long TotalRecv = 0;
    double InitTime = 0;
    double LastPacket = 0;
    bool Alive = true;
};


class GenericNetWrapper {
    public:
        bool senddat(unsigned char* data, int bufsize = 1024);
        unsigned char* getdat(int bufsize = 1024);
        void closeCon();
        ConnInfo report();

        void setPriority(int priority);

    protected:
        ConnInfo info;
        unsigned char* buffer = nullptr;
        int sockfd;
};