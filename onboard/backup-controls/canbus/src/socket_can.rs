use std::collections::HashMap;

use socketcan::{BlockingCan, CanDataFrame, CanFilter, CanFrame, CanSocket, EmbeddedFrame, NonBlockingCan, Socket, SocketOptions};

use crate::{can_v3::{CANHeader, CANPacketHandler, DeviceID, GroupID, data_to_f32, data_to_i8, data_to_i16}, packets::OutboundCANPacketV3};



pub struct SocketCAN {
    inner: CanSocket,
    handlers: HashMap<CANHeader, CANPacketHandler>,
}

impl SocketCAN {
    pub fn new(interface: &str) -> Result<Self, socketcan::Error> {
        let socket = CanSocket::open(interface)?;

        socket.set_nonblocking(true)?;

        // loopback enables receiving msgs from other processes on this system
        // but then we disable receiving msgs from this process
        socket.set_loopback(true)?;
        socket.set_recv_own_msgs(false)?;

        // by default, ignore all incoming traffic
        socket.set_filter_drop_all()?;

        // TODO: should we enable timestamping?

        return Ok(SocketCAN {
            inner: socket,
            handlers: HashMap::new(),
        });
    }



    /// Registers a callback for a specific id and command pair
    /// 
    /// If callback already exists, then it is overridden
    pub fn register_listener(&mut self, filter: CANHeader, callback: CANPacketHandler) -> Result<(), std::io::Error> {
        if callback.command_id() != filter.get_command_id() {
            return Err(std::io::Error::other("incorrect command id"));
        }

        self.handlers.insert(filter, callback);

        // add header to the list of CAN id filters
        self.inner.set_filters(
            &self.handlers.keys()
            .map(|header| {
                CanFilter::new(header.inner as u32, 0xFFFFFFFF_u32)
            }).collect::<Vec<CanFilter>>()
        )?;
        Ok(())
    }



    /// Will try to transmit non-blocking. If that fails, try again in blocking mode
    pub fn send<P>(&mut self, group_id: GroupID, device_id: DeviceID, packet: &P) -> Result<(), socketcan::Error> where P: OutboundCANPacketV3 {
        let header = CANHeader::new(group_id, device_id, P::COMMAND_ID).inner;
        let data = packet.encode();

        let frame = &CanFrame::Data(
            CanDataFrame::new(
                socketcan::Id::Standard(socketcan::StandardId::new(header)
                    .ok_or(socketcan::Error::Io(std::io::Error::other("invalid CAN header, more than 11 bits used")))?),
                &data
            ).ok_or(socketcan::Error::Io(std::io::Error::other("failed to create CAN frame, data too long")))?
        );
        
        // try non-blocking transmit, otherwise do blocking transmit
        match NonBlockingCan::transmit(&mut self.inner, frame) {
            Ok(_) => {},
            Err(nb::Error::WouldBlock) => {
                eprintln!("warning: sending message in blocking mode");
                BlockingCan::transmit(&mut self.inner, frame)?;
            },
            Err(nb::Error::Other(e)) => { return Err(e); }
        };

        return Ok(());
    }



    /// If `Ok(true)`, we successfully received and handled a message
    /// 
    /// If `Ok(false)`, we did not receive a message
    pub fn read_msg(&mut self) -> Result<bool, socketcan::Error> {
        let frame = self.inner.read_raw_frame();
        let received_msg = frame.is_ok();

        if let Ok(frame) = frame {
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
        } else {
            dbg!(&frame);
        }

        return Ok(received_msg);
    }
}

