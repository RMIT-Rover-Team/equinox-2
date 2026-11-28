#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <span>

#include "common/VelocityActuator.h"
#include "PaverMagnet.h"
#include "universalcan/RoverCanMaster.h"

class Excavator {
private:
    RoverCanMaster &m;
public:
    VelocityActuator boom_tilt;
    VelocityActuator rock_bucket_tilt;
    VelocityActuator sand_bucket_tilt;
    PaverMagnet paver_magnet;

    Excavator(RoverCanMaster &can_master);
    ~Excavator();
    void estop();

    /// Latest position broadcast (BrInt16, slot 0) by a device, 0 until one arrives.
    int16_t get_position(uint8_t device_id) const;

private:
    // The master's handlers are plain function pointers, so they can't capture
    // `this`; this static callback reaches the live Excavator through `instance`.
    static void handle_position(uint8_t group_id, uint8_t device_id,
                                std::span<const int16_t, 4> values);
    static inline std::atomic<Excavator *> instance{nullptr};

    std::array<std::atomic<int16_t>, 32> positions{};   // indexed by device ID
};
