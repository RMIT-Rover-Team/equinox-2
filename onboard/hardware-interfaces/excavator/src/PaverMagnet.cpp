#include "PaverMagnet.h"

PaverMagnet::PaverMagnet(uint8_t device_id, RoverCanMaster &can_master) : device_id(device_id), can_master(can_master), status(false) {}

PaverMagnet::~PaverMagnet() {}

void PaverMagnet::set_status(bool status) {
    if (status) spdlog::info("Enabled paver magnet");
    else spdlog::info("Disabled paver magnet");

    this->status = status;

    int8_t msg[8] = { (int8_t)status, 0, 0, 0, 0, 0, 0, 0 };
    can_master.tx_int8(GroupId::PAYLOAD, device_id, msg);
}

bool PaverMagnet::get_status() {
    return status;
}