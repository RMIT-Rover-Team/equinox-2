#include "PaverMagnet.h"

PaverMagnet::PaverMagnet(uint8_t device_id, RoverCanMaster &can_master) : device_id(device_id), can_master(can_master), status(false) {}

PaverMagnet::~PaverMagnet() {}

bool PaverMagnet::get_status() {
    return status;
}

void PaverMagnet::set_status(bool status) {
    if (status) spdlog::info("Enabled paver magnet");
    else spdlog::info("Disabled paver magnet");

    this->status = status;

    int8_t msg[8] = { (int8_t)status, 0, 0, 0, 0, 0, 0, 0 };
    can_master.tx_int8(GroupId::PAYLOAD, device_id, msg);
}

void PaverMagnet::estop() {
    spdlog::critical("ESTOP PAVER MAGNET {0:x}", device_id);
    can_master.estop(GroupId::PAYLOAD, device_id);
}

void PaverMagnet::ping() {
    can_master.ping(GroupId::PAYLOAD, device_id);
}