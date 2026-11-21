/*
 * Heater.h
 * Defines the Heater hardware abstraction.
 * Tracks target/current temperature and exposes methods for setting
 * and reading heater temperature state.
 */

#ifndef EQUINOX_2_HEATER_H
#define EQUINOX_2_HEATER_H

#include <universalcan/GenericCan.h>
#include <universalcan/RoverCanMaster.h>
#include "CanDevice.h"

class Heater : public CanDevice {
private:
    int16_t target_temperature;
    int16_t current_temperature;
protected:
    void update_current(int16_t current) override;
public:
    Heater(uint8_t device_id, RoverCanMaster& can_master);
    ~Heater() override = default;
    void set_target_temperature(int16_t target);
    int16_t get_current_temperature() const;
    int16_t get_target_temperature() const;
};


#endif //EQUINOX_2_HEATER_H
