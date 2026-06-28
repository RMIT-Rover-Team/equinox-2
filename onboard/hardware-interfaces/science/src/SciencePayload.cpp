/*
 * SciencePayload.cpp
 * Implements the top-level science payload interface.
 * Creates teh CAN bus connection, CAN master and all science subsystem devices:
 * heater, drill, microscope
*/

#include "SciencePayload.h"

// TODO: Confirm device IDs with engineering
SciencePayload::SciencePayload()
    : can_bus(CAN_BUS_NAME)
    , can_master(can_bus, GroupId::PAYLOAD)
    , heater(0x01, can_master)
    , drill(0x02, can_master)
    , microscope(0x03, can_master) {}

SciencePayload::~SciencePayload() {
}