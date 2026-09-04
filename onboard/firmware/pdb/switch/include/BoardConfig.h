#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace PdbConfig {

constexpr size_t ChannelCount = 8u;

constexpr uint8_t FirstSwitchboardSegment = 2u;
constexpr uint8_t SecondSwitchboardSegment = 3u;
constexpr uint8_t FirstSwitchboardChannelCount = 6u;

constexpr uint8_t ChannelCsPb6 = 254u;  // Special pin for PB6 as Arduino does not have one
constexpr uint8_t ChannelCsPb7 = 255u;  // Special pin for PB7 as Arduino does not have one

constexpr uint8_t ChannelCsPins[ChannelCount] = {
    4u, 5u, 6u, 7u, 8u, 9u, ChannelCsPb6, ChannelCsPb7,
};

constexpr uint8_t SwitchPins[ChannelCount] = {
    A0, A1, A2, A3, A4, A5, 2u, 3u,
};

}  // namespace PdbConfig
