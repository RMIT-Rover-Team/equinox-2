#include "VCanTest.h"


GroupId get_group_id(uint32_t header) { return (GroupId)((header >> 9) & 0b11); }
uint8_t get_device(uint32_t header) { return (uint8_t)((header >> 4) & 0b11111); }
CommandId get_command_id(uint32_t header) { return (CommandId)(header & 0b1111); }

std::string get_command_name(uint16_t header) {
    CommandId cmd = get_command_id(header);
    
    switch (cmd) {
        case CommandId::PING: return "ping";
        case CommandId::ESTOP: return "estop";
        case CommandId::TXINT8: return "txint8";
        case CommandId::TXINT16: return "txint16";
        case CommandId::TXFLOAT: return "txfloat";
        case CommandId::TXDATA: return "txdata";
        default: return "unknown cmd";
    }
}

/// @brief Returns a string representation of the CAN data
/// @param cmd 
/// @param data 
/// @return 
std::string get_data(CommandId cmd, char data[CanDataLength]) {
    std::stringstream ss;

    if (cmd == CommandId::PING) return "";
    else if (cmd == CommandId::ESTOP) return "";
    else if (cmd == CommandId::TXINT8) {
        ss << ", data: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ", "
                         << data[4] << ", " << data[5] << ", " << data[6] << ", " << data[7];
        return ss.str();
    }
    else if (cmd == CommandId::TXINT16) {
        // reinterpret u8[8] to u16[4]
        int16_t data_16[CanDataLength/2];
        memcpy(data_16, data, CanDataLength);
        ss << ", data: " << data_16[0] << ", " << data_16[1] << ", " << data_16[2] << ", " << data_16[3];
        return ss.str();
    }
    else if (cmd == CommandId::TXFLOAT) {
        // reinterpret u8[8] to float[4]
        float data_f[CanDataLength/2];
        memcpy(data_f, data, CanDataLength);
        ss << ", data: " << data_f[0] << ", " << data_f[1] << ", " << data_f[2] << ", " << data_f[3];
        return ss.str();
    }
    else if (cmd == CommandId::TXDATA) {
        ss << ", data: " << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ", "
                         << data[4] << ", " << data[5] << ", " << data[6] << ", " << data[7];
        return ss.str();
    }
    else {
        ss << "UNKNOWN COMMAND ID";
        return ss.str();
    }
}


VCANTest::VCANTest() : can_bus("vcan0"), can_master(can_bus, 0x0), payload(can_master), can_reader("vcan0") {}

void VCANTest::SetUp() {
    // clear out any messages previously sent
    read_from_vcan();
    msgs_recieved = 0;
    memset(msgs, 0, sizeof(msgs));
}

void VCANTest::read_from_vcan() {
    while (can_reader.available()) {
        // ensure buffer not full
        EXPECT_LT(msgs_recieved, CANBufferLen);

        CANFrame msg = can_reader.readMSG();

        CANData data {
            get_group_id(msg.can_id),
            get_device(msg.can_id),
            get_command_id(msg.can_id),
            {0}
        };

        memcpy(data.data, msg.data, CanDataLength);

        memcpy(&msgs[msgs_recieved], &data, sizeof(CANData));
        msgs_recieved++;

        std::printf("recieved %s to device %x%s \n", get_command_name(msg.can_id).c_str(), data.device_id, get_data(get_command_id(msg.can_id), data.data).c_str());
    }
}


