/*
 * GenericMotor.h
 * Defines a generic motor abstraction for payload devices.
 * Used for motors that only need basic start/stop control.
 */

#ifndef EQUINOX_2_GENERICMOTOR_H
#define EQUINOX_2_GENERICMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>
#include <mutex>
#include <atomic>
#include "CanDevice.h"

class GenericMotor : public CanDevice{
private:
    int16_t current_rpm;
    int16_t target_rpm;
protected:
    void updateCurrent(int16_t current) override;
public:
    GenericMotor(uint8_t device_id, RoverCanMaster& can_master);
    ~GenericMotor() override = default;
    int16_t getRpm() const;
    void setRpm(int16_t target);
    void stop();
};


#endif //EQUINOX_2_GENERICMOTOR_H
