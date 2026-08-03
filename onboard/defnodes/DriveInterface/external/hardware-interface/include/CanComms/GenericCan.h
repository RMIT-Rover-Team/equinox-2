#pragma once
#include <stdint.h>

#define CanDataLength 8
struct kcan_frame {
  uint32_t can_id;
  uint8_t can_dlc;
  char data[CanDataLength];
};
typedef struct kcan_frame CANFrame;

#define MASK_ALL 0xffffffff


class GenericCan {
    public:
        virtual CANFrame readMSG() = 0;
        virtual CANFrame readMSGFrom(uint32_t Id, uint32_t Mask) = 0;
        virtual CANFrame readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms) = 0;
        virtual int writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length) = 0;
        virtual void clearBuffer() = 0;
        virtual bool available() = 0;
        virtual bool availableFrom(uint32_t Id, uint32_t Mask) = 0;
};