/*
 * GenericMotor.h
 * Defines a generic motor abstraction for payload devices.
 * Used for motors that only need basic start/stop control.
 */

#ifndef EQUINOX_2_GENERICMOTOR_H
#define EQUINOX_2_GENERICMOTOR_H

#include <universalcan/RoverCanMaster.h>
#include "CanDevice.h"

class GenericMotor : public CanDevice{
private:
    int16_t current_rpm;
    int16_t target_rpm;
protected:
    void update_current(int16_t current) override;
public:
    GenericMotor(uint8_t device_id, RoverCanMaster& can_master);
    ~GenericMotor() override = default;
    int16_t get_current_rpm() const;
    int16_t get_target_rpm() const;
    void set_rpm(int16_t target);
    void stop();
};


#endif //EQUINOX_2_GENERICMOTOR_H
