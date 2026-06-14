#include <stdint.h>
#include "../lib-universal-canbus/libuniversalcan/GenericCan.h"
#include "../lib-universal-canbus/libuniversalcan/RoverCanMaster.h"

class ExcavatorActuator {
public:
    ExcavatorActuator();
    ~ExcavatorActuator();

    double get_velocity() {
        return velocity;
    }

    void set_velocity(double target_velocity) {
        velocity = target_velocity;
    }

private:
    uint8_t id;
    double velocity;
};

ExcavatorActuator::ExcavatorActuator() {}

ExcavatorActuator::~ExcavatorActuator() {}
