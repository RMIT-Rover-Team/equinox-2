#pragma once

#include "Channel.h"

#include <stdint.h>

class EQUCAN;
class RoverCanSlave;

namespace PdbProtocol {

/**
 * @brief Sets the filter for the CAN Bus. Uses the CAN filters and CAN masks
 *
 * @param[in] can_bus The CANBus handler to pass in. For embedded programming this will be EQUCAN.
 * @return CAN filters configuration result
 */
bool configure_filters(EQUCAN& can_bus);

/**
 * @brief Convert the channel number to a CAN Bus device id in accordance with the PDB header in the Universal CAN Bus library.
 * The function does not validate if the channel id is out of range of PdbConfig::ChannelCount.
 *
 * @param[in] local_channel_id The channel id to convert to device id.
 * @returns uint8_t universal CAN Bus formatted device_id.
 */
uint8_t device_id_for_channel(uint8_t local_channel_id);

/**
 * @brief Convert the CAN Bus device id to a channel in accordance with the PDB header in the Universal CAN Bus library.
 *
 * @param[in] device_id The CAN Bus device id.
 * @param[out] local_channel_id The channel id to access the channels array.
 *
 * @return Result of parsing.
 */
bool channel_for_device_id(
    uint8_t device_id,
    uint8_t& local_channel_id
);

/**
 * @brief Sends a CAN message using the passed in raw measurements as payload.
 *
 * Uses BrInt16 command with the payload packing bus_voltage, current and temperature in that order.
 *
 * @param[in] can_slave The RoverCanSlave object.
 * @param[in] local_channel_id The channel number the measurements are from.
 * @param[in] measurements The channel's measurements to send over.
 */
void broadcast_telemetry(
    RoverCanSlave& can_slave,
    uint8_t local_channel_id,
    const Channel::RawMeasurements& measurements
);

}  // namespace PdbProtocol
