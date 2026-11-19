/* StepperMotor.h
 * Defines a stepper motor abstraction for payload mechanisms.
 * Used by Drill and Microscope to send movement commands as step counts.
 */

#ifndef EQUINOX_2_STEPPERMOTOR_H
#define EQUINOX_2_STEPPERMOTOR_H

#include <universalcan/RoverCanMaster.h>
#include "CanDevice.h"

class StepperMotor : public CanDevice {
private:
    int16_t current_steps;
    int16_t target_steps;
protected:
    void update_current(int16_t current) override;
public:
    StepperMotor(uint8_t device_id, RoverCanMaster &can_master);
    ~StepperMotor() override = default;
    void set_target_steps(int16_t steps);
    int16_t get_target_steps() const;
    int16_t get_current_steps() const;
    void stop();
};


#endif //EQUINOX_2_STEPPERMOTOR_H
