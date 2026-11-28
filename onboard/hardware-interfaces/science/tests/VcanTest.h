/*
 * VcanTest.h
 * Shared GoogleTest fixture for tests that need a CAN bus.
 * Brings up the virtual CAN interface before the first test in a suite and
 * hands out one CAN master, created on first use rather than at program
 * start, so listing tests (which the build does) never touches the interface.
 */

#ifndef EQUINOX_2_VCANTEST_H
#define EQUINOX_2_VCANTEST_H

#include <cstdlib>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>
#include <universalcan/RoverCanMaster.h>
#include <universalcan/SocketCAN.h>

#include "SciencePayload.h"

struct VcanTest : public ::testing::Test {
    // Same commands as lib-universal-canbus/linux/SetupVCAN.sh. Skipped when
    // the interface already exists, so sudo is only needed the first time.
    static void SetUpTestSuite() {
        const std::string name = CAN_BUS_NAME;
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

    static RoverCanMaster &test_can_master() {
        static SocketCAN can_bus(CAN_BUS_NAME);
        static RoverCanMaster can_master(can_bus);
        return can_master;
    }
};

#endif //EQUINOX_2_VCANTEST_H
