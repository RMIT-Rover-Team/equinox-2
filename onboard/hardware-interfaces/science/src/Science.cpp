#include "common/CanAddress.h"
#include <universalcan/SocketCAN.h>
#include <universalcan/RoverCanMaster.h>
#include <Science.h>

Science::Science(RoverCanMaster &can_master)
  : m_(can_master)
  , drill(CanAddress(GroupID::Payload, 0x0), CanAddress(GroupID::Payload, 0x03), can_master)
  , microscope(CanAddress(GroupID::Payload, 0x1), CanAddress(GroupID::Payload, 0x2), can_master)
  , heater(CanAddress(GroupID::Payload, 0x04), can_master) {
    m_.start_heartbeat(GroupID::Payload, 0x0);
    m_.start_listen();
  };
