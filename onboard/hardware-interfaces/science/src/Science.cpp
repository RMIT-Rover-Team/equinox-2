#include "common/CanAddress.h"
#include <universalcan/SocketCAN.h>
#include <universalcan/RoverCanMaster.h>
#include <Science.h>

Science::Science(RoverCanMaster &can_master)
  : m_(can_master)
  , drill(CanAddress(GroupID::Payload, 0x0), CanAddress(GroupID::Payload, 0x03), can_master)
  , microscope(CanAddress(GroupID::Payload, 0x1), CanAddress(GroupID::Payload, 0x2), can_master)
  , heater(CanAddress(GroupID::Payload, 0x04), can_master) {
    // Every handler must be set before listening starts: unset ones are
    // garbage pointers, and the listen thread calls whichever matches a frame.
    m_.handle_ping     = [](uint8_t, uint8_t) {};
    m_.handle_br_int8  = [](uint8_t, uint8_t, std::span<const int8_t, 8>) {};
    m_.handle_br_int16 = [](uint8_t, uint8_t, std::span<const int16_t, 4>) {};
    m_.handle_br_float = [](uint8_t, uint8_t, std::span<const float, 2>) {};
    m_.handle_br_data  = [](uint8_t, uint8_t, std::span<const uint8_t, 8>) {};
    m_.handle_br_data2 = [](uint8_t, uint8_t, std::span<const uint8_t, 8>) {};

    m_.start_heartbeat(GroupID::Payload, 0x0);
    m_.start_listen();
}

Science::~Science() {
    m_.stop_listen();
    m_.stop_heartbeat(GroupID::Payload, 0x0);
}
