use std::collections::VecDeque;

use socketcan::{CanAddr, CanDataFrame, CanFrame, CanId, CanInterface, CanSocket, EmbeddedFrame, BlockingCan, Socket, addr::{CAN_RAW, PF_CAN}};

use crate::can_v3::{CANHeader, CommandID, GroupID};



pub struct SocketCAN {
    inner: CanSocket,
    recv_buffer: VecDeque<socketcan::CanFrame>,

    pub handle_ping: Option<fn(CANHeader)>,
    pub handle_br_int8: Option<fn(CANHeader, &[i8; 8])>,
    pub handle_br_int16: Option<fn(CANHeader, &[i16; 4])>,
    pub handle_br_float: Option<fn(CANHeader, &[f32; 2])>,
    pub handle_br_data: Option<fn(CANHeader, &[u8; 8])>,
    pub handle_br_data2: Option<fn(CANHeader, &[u8; 8])>
}

impl SocketCAN {
    pub fn new(interface: &str) -> Result<Self, socketcan::Error> {
        let socket = CanSocket::open(interface)?;

        // TODO: should we enable timestamping?


        return Ok(SocketCAN {
            inner: socket,
            recv_buffer: VecDeque::new(),

            handle_ping: None,
            handle_br_int8: None,
            handle_br_int16: None,
            handle_br_float: None,
            handle_br_data: None,
            handle_br_data2: None
        });
    }

    // shouldnt fail:
    // data is always 8 bytes
    // and header should not be malformed (TODO: enforce this)
    // can fail:
    // sending buffer is full, so this packet is discarded
    fn send(&mut self, header: CANHeader, data: &[u8; 8]) -> Result<(), socketcan::Error> {
        let frame = &CanFrame::Data(
            CanDataFrame::new(
                socketcan::Id::Standard(socketcan::StandardId::new(header.inner)
                    .ok_or(socketcan::Error::Io(std::io::Error::other("invalid CAN header, more than 11 bits used")))?),
                data
            ).ok_or(socketcan::Error::Io(std::io::Error::other("failed to create CAN frame, data too long")))?
        );
        
        // this blocks until the send buffer has space
        self.inner.transmit(frame)?;

        return Ok(());
    }


    // send
    pub fn send_ping(&mut self, group: GroupID, id: u8) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::Ping)?;
        return self.send(header, &[0; 8]);
    }

    pub fn send_estop(&mut self, group: GroupID, id: u8) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::EStop)?;
        return self.send(header, &[0; 8]);
    }

    pub fn send_txint8(&mut self, group: GroupID, id: u8, data: &[i8; 8]) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::TxInt8)?;

        let mut bytes: [u8; 8] = [0; 8];
        for i in 0..8 {
            [bytes[i]] = data[i].to_be_bytes();
        }

        return self.send(header, &bytes);
    }

    pub fn send_txint16(&mut self, group: GroupID, id: u8, data: &[i16; 4]) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::TxInt8)?;

        let mut bytes: [u8; 8] = [0; 8];
        for i in 0..4 {
            [bytes[2*i], bytes[2*i+1]] = data[i].to_be_bytes();
        }

        return self.send(header, &bytes);
    }

    pub fn send_txfloat(&mut self, group: GroupID, id: u8, data: &[f32; 2]) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::TxInt8)?;

        let mut bytes: [u8; 8] = [0; 8];
        for i in 0..2 {
            [bytes[4*i], bytes[4*i+1], bytes[4*i+2], bytes[4*i+3]] = data[i].to_be_bytes();
        }

        return self.send(header, &bytes);
    }

    pub fn send_txdata(&mut self, group: GroupID, id: u8, data: &[u8; 8]) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::TxInt8)?;
        return self.send(header, data);
    }

    pub fn send_txdata2(&mut self, group: GroupID, id: u8, data: &[u8; 8]) -> Result<(), socketcan::Error> {
        let header = CANHeader::new(group, id, CommandID::TxInt8)?;
        return self.send(header, data);
    }



    pub fn poll(&mut self) {
        if let Ok(frame) = self.inner.read_raw_frame() {
            // TODO: proper err handling
            let header: CANHeader = match frame.can_id.try_into() {
                Ok(h) => h,
                Err(e) => {
                    eprintln!("{:?}", e);
                    return;
                }
            };

            match header.get_command_id() {
                CommandID::Ping => {
                    if let Some(f) = self.handle_ping {
                        f(header);
                    }
                },
                CommandID::BrInt8 => {
                    if let Some(f) = self.handle_br_int8 {
                        let data = frame.data.map(|x| x as i8);
                        f(header, &data);
                    }
                },
                CommandID::BrInt16 => {
                    if let Some(f) = self.handle_br_int16 {
                        let data = unsafe {
                            // unchecked convert to 4 2 byte chunks
                            <[[u8; 2]; 4]>::try_from(frame.data.as_chunks_unchecked::<2>()).unwrap()
                        }.map(|x| i16::from_be_bytes(x));
                        f(header, &data);
                    }
                },
                CommandID::BrFloat => {
                    if let Some(f) = self.handle_br_float {
                        let data = unsafe {
                            // unchecked convert to 2 4 byte chunks
                            <[[u8; 4]; 2]>::try_from(frame.data.as_chunks_unchecked::<4>()).unwrap()
                        }.map(|x| f32::from_be_bytes(x));
                        f(header, &data);
                    }
                },
                CommandID::BrData => {
                    if let Some(f) = self.handle_br_data {
                        f(header, &frame.data);
                    }
                },
                CommandID::BrData2 => {
                    if let Some(f) = self.handle_br_data2 {
                        f(header, &frame.data);
                    }
                },
                x => {
                    eprintln!("Invalid command id recieved {:?}", x);
                    return;
                }
            }
        }
    }
}