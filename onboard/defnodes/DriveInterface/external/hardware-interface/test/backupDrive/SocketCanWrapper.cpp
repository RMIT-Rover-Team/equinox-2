#include "SocketCanWrapper.h"
#include <stdint.h>
#include <fcntl.h>
#include <chrono>

//debug data dump
void dumpBuffer(uint8_t* buffer, int size) {
    for (int i = 0; i < size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}
void dumpBuffer(char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        printf("%02x ", (uint8_t)buffer[i]);
    }
    printf("\n");
  }


WrappedCANBus::WrappedCANBus(const char* interfaceName){
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Open Socket");
		throw 1;
	}

    //Copy interface name and tell the CANBus socket about it
    strcpy(ifr.ifr_name, interfaceName);
    ioctl(s, SIOCGIFINDEX, &ifr);

    //Clear and assign the address
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

    //Bind the socket
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind Socket");
		throw 2;
	}

}

CANFrame WrappedCANBus::readFromSocket() {
    CANFrame temp_frame;

    struct can_frame rawFrame;
    int nbytes = read(s, &rawFrame, sizeof(can_frame));

    //printf("RIN << ");dumpBuffer(rawFrame.data, rawFrame.can_dlc);

    if (nbytes < 0) {
        perror("Socket Read Error");
        return {}; // Return empty frame on error
    }
    if (nbytes < sizeof(CANFrame)) {
        fprintf(stderr, "Incomplete CAN frame read from socket\n");
        return {};
    }

    temp_frame.can_id &= 0xFFF;

    //Convert to CANFrame
    temp_frame.can_id = rawFrame.can_id;
    temp_frame.can_dlc = rawFrame.can_dlc;
    memcpy(temp_frame.data, rawFrame.data, rawFrame.can_dlc);

    //printf("IN << ");dumpBuffer(temp_frame.data, temp_frame.can_dlc);

    return temp_frame;
}

CANFrame WrappedCANBus::readMSG() {
    if (!user_buffer.empty()) {
        CANFrame frame_from_buffer = user_buffer.front();
        user_buffer.pop_front();
        return frame_from_buffer;
    }

    return readFromSocket();
}

int WrappedCANBus::writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length) {
    struct can_frame local_frame;
    local_frame.can_id = IdAndFlags;
    local_frame.can_dlc = (length > 8) ? 8: length; //We only support 8 byte packets

    memset(local_frame.data, 0, 8);
    memcpy(local_frame.data, data, length);

    //printf("OUT >> ");dumpBuffer(local_frame.data, local_frame.can_dlc);

    return write(s, &local_frame, sizeof(can_frame)) != sizeof(can_frame);
}

CANFrame WrappedCANBus::readMSGFrom(uint32_t Id, uint32_t Mask) {
    for (auto it = user_buffer.begin(); it != user_buffer.end(); ++it) {
        if ((it->can_id & Mask) == (Id & Mask)) {
            CANFrame found_frame = *it;
            user_buffer.erase(it);
            return found_frame;
        }
    }

    while (true) {
        CANFrame new_frame = readFromSocket();

        if (new_frame.can_id == 0 && new_frame.can_dlc == 0) {
            printf("Bad CAN Packet\n");
            //return {};
        }
        
        if ((new_frame.can_id & Mask) == (Id & Mask)) {
            return new_frame;
        } else {
            user_buffer.push_back(new_frame);
        }
    }
}

CANFrame WrappedCANBus::readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms) {
    for (auto it = user_buffer.begin(); it != user_buffer.end(); ++it) {
        if ((it->can_id & Mask) == (Id & Mask)) {
            CANFrame found_frame = *it;
            user_buffer.erase(it);
            return found_frame;
        }
    }

    //Establish the start time
    auto expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

    while (true){
        //printf("Check Avail\n");
        if (this->availableFrom(Id, Mask)){

            //Fetch it from the buffer
            for (auto it = user_buffer.begin(); it != user_buffer.end(); ++it) {
                if ((it->can_id & Mask) == (Id & Mask)) {
                    CANFrame found_frame = *it;
                    user_buffer.erase(it);
                    return found_frame;
                }
            }
        }

        //determine if time has expired
        if (std::chrono::steady_clock::now() >= expiry) { // expired }
            printf("Timeout awaiting %02x\n",Id);
            return {};
        }
    }
}

void WrappedCANBus::clearBuffer() {
    user_buffer.clear();

    int flags = fcntl(s, F_GETFL, 0);
    if (flags == -1) return;
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
    while (read(s, &frame, sizeof(CANFrame)) > 0);
    fcntl(s, F_SETFL, flags);
}

bool WrappedCANBus::available() {
    if (!user_buffer.empty()) {
        return true;
    }

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(s, &rdfs);
    struct timeval timeout = {0, 0};
    int ret = select(s + 1, &rdfs, NULL, NULL, &timeout);
    return ret > 0;
}

bool WrappedCANBus::availableFrom(uint32_t Id, uint32_t Mask) {
    for (const auto& frame : user_buffer) {
        if ((frame.can_id & Mask) == (Id & Mask)) {
            return true;
        }
    }

    CANFrame new_frame;
    while (this->available()){
        new_frame = readFromSocket();
        user_buffer.push_back(new_frame);

        for (const auto& frame : user_buffer) {
            if ((frame.can_id & Mask) == (Id & Mask)) {
                return true;
            }
        }
    }
    
    

    return false;
}

WrappedCANBus::~WrappedCANBus(){
    close(s);
}