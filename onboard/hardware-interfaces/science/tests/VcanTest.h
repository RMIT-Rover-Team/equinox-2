/*
 * VcanTest.h
 * Shared GoogleTest fixture for tests that need a CAN bus.
 *
 * - Brings up the virtual CAN interface before the first test in a suite
 *   (skipped if it already exists, so sudo is only needed the first time).
 * - Hands out one CAN master, created on first use rather than at program
 *   start, so listing tests (which the build does) never touches the bus.
 * - Opens a fresh listener socket per test, so a test can assert on the exact
 *   frames the code under test put on the bus.
 */
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <common/CanAddress.h>
#include <universalcan/CanID.h>
#include <universalcan/CanTypes.h>
#include <universalcan/RoverCanMaster.h>
#include <universalcan/SocketCAN.h>

inline constexpr const char *TEST_CAN_BUS = "vcan0";

struct VcanTest : public ::testing::Test {
    // Same commands as lib-universal-canbus/linux/SetupVCAN.sh.
    static void SetUpTestSuite() {
        const std::string name = TEST_CAN_BUS;
        if (std::filesystem::exists("/sys/class/net/" + name)) {
            return;
        }
        const std::string commands[] = {
            "sudo modprobe vcan",
            "sudo ip link add dev " + name + " type vcan",
            "sudo ip link set up " + name,
        };
        for (const std::string &command : commands) {
            ASSERT_EQ(0, std::system(command.c_str()))
                << "Could not set up " << name << ": " << command;
        }
    }

    // Opened before each test body runs, so it sees every frame the test sends.
    void SetUp() override { listener = std::make_unique<SocketCAN>(TEST_CAN_BUS); }

    static RoverCanMaster &test_can_master() {
        static SocketCAN can_bus(TEST_CAN_BUS);
        static RoverCanMaster m(can_bus);
        return m;
    }

    // Next frame sent to `addr` with `command`; can_dlc is 0 if none arrives in time.
    CANFrame expect_frame(CanAddress addr, CommandID command, uint32_t timeout_ms = 500) {
        const uint16_t id = HeaderUtils::encode(
            Header{addr.group, addr.device, static_cast<uint8_t>(command)});
        return listener->readMSGFrom(id, HeaderMask::NoMask, timeout_ms);
    }

    static int16_t int16_at(const CANFrame &frame, std::size_t index) {
        int16_t value;
        std::memcpy(&value, frame.data + index * sizeof(int16_t), sizeof(value));
        return value;
    }

    std::unique_ptr<SocketCAN> listener;
};
