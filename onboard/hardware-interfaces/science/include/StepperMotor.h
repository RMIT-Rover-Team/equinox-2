/* StepperMotor.h
 * Defines a stepper motor abstraction for payload mechanisms.
 * Used by Drill and Microscope to send movement commands as step counts.
 */
 
#ifndef EQUINOX_2_STEPPERMOTOR_H
#define EQUINOX_2_STEPPERMOTOR_H

#include "../../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"
#include <thread>
#include "CanSender.h"

class StepperMotor : public CanSender {
private:
    int16_t current_steps;
    int16_t target_steps;
protected:
    void setCurrent(int16_t current) override;
public:
    StepperMotor(uint8_t device_id, RoverCanMaster &can_master);
    ~StepperMotor();
    void setTargetSteps(int16_t steps);
    int16_t getTargetSteps() const;
    int16_t getCurrentSteps() const;
    void stop();
};


#endif //EQUINOX_2_STEPPERMOTOR_H
