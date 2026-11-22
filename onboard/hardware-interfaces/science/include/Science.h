/*
 * SciencePayload.h
 * Defines the SciencePayload class.
 * This is the main entry point for the science payload hardware interface.
 * It owns the CAN bus, CAN master and science device abstractions.
 */
#pragma once

#include <universalcan/SocketCAN.h>
#include <universalcan/RoverCanMaster.h>
#include "Heater.h"
#include "Drill.h"
#include "Microscope.h"

class Science {
private:
    RoverCanMaster &m_;
public:
    Drill drill;
    Microscope microscope;
    Heater heater;

    Science(RoverCanMaster &can_master);
    ~Science();
};
