/*
 * SciencePayload.h
 * Defines the SciencePayload class.
 * This is the main entry point for the science payload hardware interface.
 * It owns the CAN bus, CAN master and science device abstractions.
 */
 
#include "../../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../../lib-universal-canbus/libuniversalcan/SocketCanWrapper.h"
#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "Heater.h"
#include "Drill.h"
#include "Microscope.h"

#ifndef EQUINOX_2_SCIENCEPAYLOAD_H
#define EQUINOX_2_SCIENCEPAYLOAD_H

#define CAN_BUS_NAME "can0"

class SciencePayload {
private:
    WrappedCANBus can_bus;
    RoverCanMaster can_master;
    Heater heater;
    Drill drill;
    Microscope microscope;
public:
    SciencePayload();
    ~SciencePayload();
};


#endif //EQUINOX_2_SCIENCEPAYLOAD_H
