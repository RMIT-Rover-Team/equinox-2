/*
 * Heater.h
 * Defines the Heater hardware abstraction.
 * Tracks target/current temperature and exposes methods for setting
 * and reading heater temperature state.
 */

#pragma once

#include <common/CanAddress.h>
#include <universalcan/GenericCan.h>
#include <universalcan/RoverCanMaster.h>

class Heater {
private:
    RoverCanMaster &m_;
    CanAddress addr_;
    bool status_ = false;
public:
    Heater(const CanAddress addr, RoverCanMaster& m);
    ~Heater() = default;
    void set_heater_status(bool status);
    bool get_heater_status() const;
};
