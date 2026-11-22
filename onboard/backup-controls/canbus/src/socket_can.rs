use socket2::Socket;
use socketcan::{CanAddr, addr::{CAN_RAW, PF_CAN}};



pub struct SocketCAN {
    pub addr: CanAddr,
    pub socket: Socket
}

impl SocketCAN {
    pub fn new(interface: &str) -> Result<Self, socketcan::Error> {
        let addr = socketcan::CanAddr::from_iface(&interface)?;

        let socket = socket2::Socket::new_raw(
            socket2::Domain::from(PF_CAN),
            socket2::Type::DGRAM,   // raw
            Some(socket2::Protocol::from(CAN_RAW))
        )?;

        socket.bind(&addr.into_sock_addr())?;

        return Ok(Self {
            addr: addr,
            socket: socket
        });
    }
}