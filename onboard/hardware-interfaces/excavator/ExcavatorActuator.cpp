#include <stdint.h>
#include "../GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
public:
    ExcavatorActuator(u_int8_t id, GenericCan& can) : velocity(0.0), can_master(can, id) {}
    ~ExcavatorActuator();

    double get_velocity() {
        return velocity;
    }

    void set_velocity(double target_velocity) {
        velocity = target_velocity;
    }

private:
    double velocity;
    RoverCanMaster can_master;
};

ExcavatorActuator::~ExcavatorActuator() {}
