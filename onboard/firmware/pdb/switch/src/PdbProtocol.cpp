#include "PdbProtocol.h"

#include "BoardConfig.h"

#include <stddef.h>
#include <universalcan/CanId.h>
#include <universalcan/CanTypes.h>
#include <universalcan/EQUCAN.h>
#include <universalcan/RoverCanSlave.h>

namespace {

constexpr uint32_t SegmentTxInt8Mask =
    HeaderMask::GroupMask |
    (0x18u << HeaderLayout::CmdIdSize) |
    HeaderMask::CmdMask;

constexpr uint32_t make_filter(
    GroupID group,
    uint8_t id_segment,
    CommandID command
) {
  const uint8_t device_id = (id_segment & 0x03u) << 3u;

  return HeaderUtils::encode(Header{
      static_cast<uint8_t>(group),
      device_id,
      static_cast<uint8_t>(command),
  });
}

constexpr uint32_t CanFilters[] = {
    make_filter(Onboard, PdbConfig::FirstSwitchboardSegment, TxInt8),
    make_filter(Onboard, PdbConfig::SecondSwitchboardSegment, TxInt8),
};

constexpr uint32_t CanMasks[] = {
    SegmentTxInt8Mask,
    SegmentTxInt8Mask,
};

constexpr size_t FilterCount =
    sizeof(CanFilters) / sizeof(CanFilters[0]);

static_assert(
    FilterCount == sizeof(CanMasks) / sizeof(CanMasks[0]),
    "Every CAN filter must have a corresponding mask"
);

}  // namespace

namespace PdbProtocol {

bool configure_filters(EQUCAN& can_bus) {
  return can_bus.set_socket_filter(CanFilters, CanMasks, FilterCount);
}

uint8_t device_id_for_channel(uint8_t local_channel_id) {
  if (local_channel_id < PdbConfig::FirstSwitchboardChannelCount) {
    return
        (PdbConfig::FirstSwitchboardSegment << 3u) |
        (local_channel_id & 0x07u);
  }

  return
      (PdbConfig::SecondSwitchboardSegment << 3u) |
      (local_channel_id - PdbConfig::FirstSwitchboardChannelCount);
}

bool channel_for_device_id(
    uint8_t device_id,
    uint8_t& local_channel_id
) {
  const uint8_t segment = (device_id & 0x18u) >> 3u;
  const uint8_t segment_channel = device_id & 0x07u;

  if (segment == PdbConfig::FirstSwitchboardSegment) {
    if (segment_channel >= PdbConfig::FirstSwitchboardChannelCount) {
      return false;
    }

    local_channel_id = segment_channel;
    return true;
  }

  if (segment == PdbConfig::SecondSwitchboardSegment) {
    local_channel_id = PdbConfig::FirstSwitchboardChannelCount + segment_channel;
    return local_channel_id < PdbConfig::ChannelCount;
  }

  return false;
}

void broadcast_telemetry(
    RoverCanSlave& can_slave,
    uint8_t local_channel_id,
    const Channel::RawMeasurements& measurements
) {
  const int16_t payload[4] = {
      static_cast<int16_t>(measurements.bus_voltage),
      measurements.current,
      measurements.temperature,
      0,
  };

  can_slave.br_int16(
      GroupID::Onboard,
      device_id_for_channel(local_channel_id),
      payload
  );
}

}  // namespace PdbProtocol
