#pragma once

#include <stdint.h>
#include "GenericCan.h"

#define RATCAN_Speed CAN_125KBPS
#define RATCAN_Datalen CanDataLength //The size of Canbus messages
#define RATCAN_Queue_Size 64 // The number of "saved" messages awaiting read
#define RATCAN_SPI_CS_PIN 21
#define RATCAN_SPI_INT_PIN 7
#define CAN_RTR 0x40000000

//#define RATCAN_USEISR
//#define RATCAN_DEBUG

class RATCAN: public GenericCan {
    private:
      int CS_PIN = 21;
      CANFrame myFrame;

      CANFrame msgQueue[RATCAN_Queue_Size];
      int msgQueuePointer = 0;
      bool msgQueueFlags[RATCAN_Queue_Size];

      void dumpBuffer();

    public:
        RATCAN();

        CANFrame readMSG() override;
        CANFrame readMSGFrom(uint32_t Id, uint32_t Mask) override;
        CANFrame readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms) override;

        int writeMSG_RTR(uint32_t IdAndFlags, const char* data, uint8_t length);
        int writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length) override;

        void clearBuffer() override;

        bool available() override;
        bool availableFrom(uint32_t Id, uint32_t Mask) override;

        ~RATCAN();
};