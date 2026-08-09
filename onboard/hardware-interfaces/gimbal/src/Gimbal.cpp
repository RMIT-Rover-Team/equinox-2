#include "Gimbal.h"



Gimbal::Gimbal() : servo(0x01, RoverCanMaster::get_instance()) {}
Gimbal::~Gimbal() {}
