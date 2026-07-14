/*
 * Heater.h
 * Defines the Heater hardware abstraction.
 * Tracks target/current temperature and exposes methods for setting
 * and reading heater temperature state.
 */
 
#ifndef EQUINOX_2_HEATER_H
#define EQUINOX_2_HEATER_H

#include "../../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include "CanSender.h"

class Heater : public CanSender {
private:
    int16_t target_temperature;
    int16_t current_temperature;
protected:
    void setCurrent(int16_t current) override;
public:
    Heater(uint8_t device_id, RoverCanMaster& can_master);
    ~Heater();
    void setTargetTemperature(int16_t target);
    int16_t getCurrentTemperature() const;
    int16_t getTargetTemperature() const;
};


#endif //EQUINOX_2_HEATER_H
