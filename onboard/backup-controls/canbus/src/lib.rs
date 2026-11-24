
pub mod socket_can;
pub mod can_v3;

#[cfg(test)]
mod tests {
    use std::time::Duration;

    use crate::{can_v3::{CANHeader, CANPacketHandler}, socket_can::SocketCAN};

    use super::*;

    #[test]
    fn send_ping() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.send_ping(can_v3::GroupID::Onboard, 0b01111).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x0F0
    }

    #[test]
    fn send_estop() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.send_estop(can_v3::GroupID::Onboard, 0b01010).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x0A1
    }

    #[test]
    fn no_loopback() {
        let mut socket: SocketCAN = SocketCAN::new("vcan0").expect("failed to init canbus");

        socket.register_listener(
            CANHeader::new(can_v3::GroupID::Onboard, 0, can_v3::CommandID::Ping).expect("failed to create can header"),
            CANPacketHandler::Ping(|_header| {
                panic!("recieved ping that we sent ourselves");
            })
        ).expect("failed to register listener");

        socket.send_ping(can_v3::GroupID::Onboard, 0).expect("failed to send ping");
        // should see empty packet sent with id (header) 0x1F0

        // check for recieved msgs
        std::thread::sleep(Duration::from_millis(1000));
        socket.read_msg().unwrap();
    }
}
