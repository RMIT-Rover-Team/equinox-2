use std::collections::{HashMap, VecDeque};

use socketcan::{CanAddr, CanDataFrame, CanFrame, CanId, CanInterface, CanSocket, EmbeddedFrame, BlockingCan, Socket, addr::{CAN_RAW, PF_CAN}};

use crate::{can_v3::{CANHeader, CommandID, GroupID}, packet::{CANPacketHandler, data_to_f32, data_to_i8, data_to_i16}};



pub struct SocketCAN {
    inner: CanSocket,

    handlers: HashMap<CANHeader, CANPacketHandler>,
}

impl SocketCAN {
    pub fn new(interface: &str) -> Result<Self, socketcan::Error> {
        let socket = CanSocket::open(interface)?;

        // TODO: should we enable timestamping?


        return Ok(SocketCAN {
            inner: socket,

            handlers: HashMap::new(),
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





    /// Registers a callback for a specific id and command pair
    /// If callback already exists, then it is overridden
    pub fn register_listener(&mut self, filter: CANHeader, callback: CANPacketHandler) -> Result<(), std::io::Error> {
        if callback.command_id() != filter.get_command_id() {
            return Err(std::io::Error::other("incorrect command id"));
        }

        self.handlers.insert(filter, callback);
        Ok(())
    }

    pub fn read_msg(&mut self) -> Result<(), socketcan::Error> {
        if let Ok(frame) = self.inner.read_raw_frame() {
            let header = CANHeader::try_from(frame.can_id)
                .map_err(|e| socketcan::Error::Io(e))?;

            match self.handlers.get(&header) {
                Some(CANPacketHandler::Ping(f)) => f(header),
                Some(CANPacketHandler::BrInt8(f)) => f(header, *data_to_i8(&frame.data)?),
                Some(CANPacketHandler::BrInt16(f)) => f(header, *data_to_i16(&frame.data)?),
                Some(CANPacketHandler::BrFloat(f)) => f(header, *data_to_f32(&frame.data)?),
                Some(CANPacketHandler::BrData(f)) => f(header, frame.data),
                Some(CANPacketHandler::BrData2(f)) => f(header, frame.data),
                _ => {}
            }
        }

        Ok(())
    }
}

