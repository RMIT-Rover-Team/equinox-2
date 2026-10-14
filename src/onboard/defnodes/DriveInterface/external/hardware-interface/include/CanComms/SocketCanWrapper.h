#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <deque>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "GenericCan.h"

#define CAN_REQUEST CAN_RTR_FLAG

typedef struct kcan_frame CANFrame;

class WrappedCANBus: public GenericCan {
    private:
        int s, i; 

        struct sockaddr_can addr;
        struct ifreq ifr;
        CANFrame frame;
        std::deque<CANFrame> user_buffer;

        CANFrame readFromSocket();
    public:
        WrappedCANBus(const char* interfaceName);
        
        CANFrame readMSG() override;
        CANFrame readMSGFrom(uint32_t Id, uint32_t Mask) override;
        CANFrame readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms) override;

        int writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length) override;

        void clearBuffer() override;

        bool available() override;
        bool availableFrom(uint32_t Id, uint32_t Mask) override;

        void disableLoopback();

        ~WrappedCANBus();
};