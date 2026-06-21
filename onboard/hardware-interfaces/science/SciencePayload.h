#include "GenericCan.h"
#include "SocketCanWrapper.h"
#include "RoverCanMaster.h"

#ifndef EQUINOX_2_SCIENCEPAYLOAD_H
#define EQUINOX_2_SCIENCEPAYLOAD_H

class SciencePayload {
private:
    uint8_t heater_temperature;
    float drill_position;
    float drill_velocity;
    float microscope_position;
    float microscope_rotation;
public:
    SciencePayload();
    ~SciencePayload();
    void motor_cycle(); // Cycle motor from min to max to find limits
    void set_heater_state(uint8_t temperature);
    void set_heater_state(bool state);
    int get_heater_state();
    int get_heater_current_temperature();
    void set_servo_position(int device_id, float cm);
    float get_servo_position(int device_id);
    void set_servo_velocity(int device_id, float cm_per_second);
    float get_servo_velocity(int device_id);
    void set_microscope_swivel(int device_id, float degrees);
    float get_microscope_swivel(int device_id);
    void stop_all();
};


#endif //EQUINOX_2_SCIENCEPAYLOAD_H
