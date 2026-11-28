/*
 * SciencePayload.h
 * Defines the SciencePayload class.
 * This is the main entry point for the science payload hardware interface.
 * It owns the CAN bus, CAN master and science device abstractions.
 */

#include <universalcan/GenericCan.h>
#include <universalcan/SocketCAN.h>
#include <universalcan/RoverCanMaster.h>
#include "Heater.h"
#include "Drill.h"
#include "Microscope.h"

#ifndef EQUINOX_2_SCIENCEPAYLOAD_H
#define EQUINOX_2_SCIENCEPAYLOAD_H

#define CAN_BUS_NAME "vcan0"

struct SciencePayload {
private:
    SocketCAN can_bus;
    RoverCanMaster can_master;
public:
    Heater heater;
    Drill drill;
    Microscope microscope;

    SciencePayload()
      : can_bus(CAN_BUS_NAME)
      , can_master(can_bus)
      , heater(0x01, can_master)
      , drill(0x02, can_master)
      , microscope(0x03, can_master) {}
    ~SciencePayload();
};


#endif //EQUINOX_2_SCIENCEPAYLOAD_H
