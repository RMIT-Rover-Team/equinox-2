
pub mod socket_can;
pub mod can_v3;
pub mod packets;

#[cfg(test)]
mod tests {
    use std::time::Duration;

    use crate::{can_v3::{CANHeader, CANPacketHandler, DeviceID, GroupID}, packets::{CANV3EStop, CANV3Ping}, socket_can::SocketCAN};

    use super::*;

    #[test]
    fn send_ping() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.send(GroupID::Onboard, DeviceID::try_from(0b01111).unwrap(), &CANV3Ping).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x0F0
    }

    #[test]
    fn send_estop() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.send(GroupID::Onboard, DeviceID::try_from(0b01111).unwrap(), &CANV3EStop).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x0F1
    }

    #[test]
    fn no_loopback() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.register_listener(
            CANHeader::new(can_v3::GroupID::Onboard, can_v3::DeviceID::try_from(0).unwrap(), can_v3::CommandID::Ping),
            CANPacketHandler::Ping(|_header| {
                panic!("recieved ping that we sent ourselves");
            })
        ).expect("failed to register listener");

        socket.send(GroupID::Onboard, DeviceID::try_from(0).unwrap(), &CANV3Ping).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x000

        // check for recieved msgs
        std::thread::sleep(Duration::from_millis(1000));
        assert_eq!(socket.read_msg().unwrap(), false);
    }
}
