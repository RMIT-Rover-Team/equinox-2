#include "Gimbal.h"



Gimbal::Gimbal(RoverCanMaster& can_master) : can_master(can_master), servo(0x01, can_master) {}
Gimbal::~Gimbal() {}
