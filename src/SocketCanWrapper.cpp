#include "SocketCanWrapper.h"
#include <stdint.h>
#include <fcntl.h>


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

    can_frame rawFrame;
    int nbytes = read(s, &rawFrame, sizeof(can_frame));

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
    memcpy(temp_frame.data, rawFrame.data, CanDataLength);

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
    can_frame local_frame;
    local_frame.can_id = IdAndFlags;
    local_frame.can_dlc = (length > 8) ? 8: length; //We only support 8 byte packets

    memset(local_frame.data, 0, 8);
    memcpy(local_frame.data, data, length);

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
            return {};
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

    struct timeval start_time, current_time;
    gettimeofday(&start_time, NULL);

    while (true) {
        gettimeofday(&current_time, NULL);
        long elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                          (current_time.tv_usec - start_time.tv_usec) / 1000;

        if (timeout_ms != 0 && elapsed_ms >= timeout_ms) {
            return {};
        }

        fd_set rdfs;
        FD_ZERO(&rdfs);
        FD_SET(s, &rdfs);

        struct timeval select_timeout;
        if (timeout_ms != 0) {
            long remaining_ms = timeout_ms - elapsed_ms;
            select_timeout.tv_sec = remaining_ms / 1000;
            select_timeout.tv_usec = (remaining_ms % 1000) * 1000;
        }

        int ret = select(s + 1, &rdfs, NULL, NULL, (timeout_ms == 0) ? NULL : &select_timeout);

        if (ret < 0) {
            perror("select error");
            return {}; // Error
        }

        if (ret == 0) {
            return {}; // Timeout
        }

        CANFrame new_frame = readFromSocket();

        if (new_frame.can_id == 0 && new_frame.can_dlc == 0) {
            continue;
        }

        if ((new_frame.can_id & Mask) == (Id & Mask)) {
            return new_frame;
        } else {
            user_buffer.push_back(new_frame);
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

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(s, &rdfs);
    struct timeval timeout = {0, 0};

    int ret = select(s + 1, &rdfs, NULL, NULL, &timeout);

    if (ret > 0) {
        int flags = fcntl(s, F_GETFL, 0);
        if (flags == -1) return false;
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
        
        CANFrame new_frame;
        while ((new_frame = readFromSocket()).can_dlc > 0) {
             user_buffer.push_back(new_frame);
        }

        fcntl(s, F_SETFL, flags);
    }
    
    for (const auto& frame : user_buffer) {
        if ((frame.can_id & Mask) == (Id & Mask)) {
            return true;
        }
    }

    return false;
}

WrappedCANBus::~WrappedCANBus(){
    close(s);
}