#include "BoardConfig.h"
#include "Channel.h"
#include "PdbProtocol.h"

#include <Arduino.h>
#include <SPI.h>
#include <universalcan/EQUCAN.h>
#include <universalcan/RoverCanSlave.h>

namespace {

Channel channels[PdbConfig::ChannelCount] = {
    {PdbConfig::ChannelCsPins[0], PdbConfig::SwitchPins[0]},
    {PdbConfig::ChannelCsPins[1], PdbConfig::SwitchPins[1]},
    {PdbConfig::ChannelCsPins[2], PdbConfig::SwitchPins[2]},
    {PdbConfig::ChannelCsPins[3], PdbConfig::SwitchPins[3]},
    {PdbConfig::ChannelCsPins[4], PdbConfig::SwitchPins[4]},
    {PdbConfig::ChannelCsPins[5], PdbConfig::SwitchPins[5]},
    {PdbConfig::ChannelCsPins[6], PdbConfig::SwitchPins[6]},
    {PdbConfig::ChannelCsPins[7], PdbConfig::SwitchPins[7]},
};

RoverCanSlave* can_slave = nullptr;

/**
 * @brief Toggles a channel on or off. This function is the hook for tx_int8 commands coming from the master.
 *
 * @param[in] device_id The device id to toggle, using the universal CAN Bus protocol.
 * @param[in] values The array of values that is passed in through callback. Only values[0] is read, 0 disabling and 1 enabling the channel and any other values being ignored
 */
void handle_toggle(uint8_t device_id, const int8_t values[8]) {
  if (values == nullptr || (values[0] != 0 && values[0] != 1)) {
    return;
  }

  uint8_t local_channel_id;
  if (!PdbProtocol::channel_for_device_id(device_id, local_channel_id)) {
    return;
  }

  if (values[0] == 1) {
    channels[local_channel_id].enable();
  } else {
    channels[local_channel_id].disable();
  }
}

/**
 * @brief Prints a channel's measurements.
 *
 * @param[in] channel_id The channel id to print.
 * @param[in] measurements The measurements to print.
 */
void print_measurements(
    uint8_t channel_id,
    const Channel::RawMeasurements& measurements
) {
  Serial.print(F("Channel "));
  Serial.print(channel_id);
  Serial.print(F(": Voltage: "));
  Serial.print(measurements.bus_voltage);
  Serial.print(F("  Current: "));
  Serial.print(measurements.current);
  Serial.print(F("  Power: "));
  Serial.print(measurements.power);
  Serial.print(F("  Temp: "));
  Serial.println(measurements.temperature);
}

}  // namespace

void setup() {
  Serial.begin(9600);
  SPI.begin();

  Serial.println(F("Initializing EQUCAN"));
  static EQUCAN equcan;

  if (!PdbProtocol::configure_filters(equcan)) {
    Serial.println(F("Failed to configure CAN filters"));
  }

  Serial.println(F("Initializing CAN slave"));
  static RoverCanSlave rover_can_slave(&equcan);
  can_slave = &rover_can_slave;
  can_slave->handle_tx_int8 = &handle_toggle;

  Serial.println(F("Initializing channels"));
  for (Channel& channel : channels) {
    channel.begin();
  }

  Serial.println(F("Enabling channels"));
  for (Channel& channel : channels) {
    channel.enable();
    delay(1000);
  }

  Serial.println(F("Ready!"));
}

void loop() {
  can_slave->noBlockListenTick();

  for (
      uint8_t channel_id = 0u;
      channel_id < PdbConfig::ChannelCount;
      channel_id++
  ) {
    Channel& channel = channels[channel_id];

    if (!channel.conversion_done()) continue;

    const Channel::RawMeasurements measurements =
        channel.read_raw_measurements();

    print_measurements(channel_id, measurements);
    PdbProtocol::broadcast_telemetry(
        *can_slave,
        channel_id,
        measurements
    );
  }

  delay(1000);
}
