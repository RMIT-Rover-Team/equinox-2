use std::{collections::HashMap, ffi::c_int, io::{Read, Write}, os::fd::AsRawFd};

use crate::{can_v3::{CANHeader, CANPacketHandler, DeviceID, GroupID, data_to_f32, data_to_i8, data_to_i16}, packets::OutboundCANPacketV3};

#[repr(transparent)]
struct CANFrame {
    inner: libc::can_frame
}

impl CANFrame {
    pub fn zeroed() -> Self {
        return unsafe { std::mem::zeroed() };
    }

    pub fn new(header: CANHeader, data: [u8; 8]) -> Self {
        let mut this = Self {
            inner: libc::can_frame::default()
        };

        this.inner.can_id = header.inner as u32;
        this.inner.data = data;
        this.inner.can_dlc = data.len() as u8;

        return this;
    }

    pub unsafe fn as_bytes(&self) -> &[u8] {
        return unsafe {
            std::slice::from_raw_parts(
                std::mem::transmute::<*const libc::can_frame, *const u8>(std::ptr::from_ref(&self.inner)),
                size_of::<libc::can_frame>()
            )
        };
    }

    pub unsafe fn as_bytes_mut(&mut self) -> &mut [u8] {
        return unsafe {
            std::slice::from_raw_parts_mut(
                std::mem::transmute::<*mut libc::can_frame, *mut u8>(std::ptr::from_mut(&mut self.inner)),
                size_of::<libc::can_frame>()
            )
        };
    }
}



pub struct SocketCAN {
    pub inner: socket2::Socket,
    handlers: HashMap<CANHeader, CANPacketHandler>,
}

impl SocketCAN {
    fn interface_name_to_socket_addr(interface: &str) -> socket2::SockAddr {
        let interface_index = unsafe {
            libc::if_nametoindex(std::ffi::CString::new(interface).unwrap().as_ptr())
        };

        let mut addr_storage: socket2::SockAddrStorage = socket2::SockAddrStorage::zeroed();
        let addr_can = unsafe { addr_storage.view_as::<libc::sockaddr_can>() };

        addr_can.can_family = libc::AF_CAN as libc::sa_family_t;
        addr_can.can_ifindex = interface_index as c_int;

        unsafe {
            return socket2::SockAddr::new(
                addr_storage,
                size_of::<libc::sockaddr_can>() as socket2::socklen_t
            );
        }  
    }


    pub fn new(interface: &str) -> Result<Self, std::io::Error> {

        // let socket = CanSocket::open(interface)?;

        // socket.set_nonblocking(true)?;

        // // loopback enables receiving msgs from other processes on this system
        // // but then we disable receiving msgs from this process
        // socket.set_loopback(true)?;
        // socket.set_recv_own_msgs(false)?;

        // // by default, ignore all incoming traffic
        // socket.set_filter_drop_all()?;

        // TODO: should we enable timestamping?

        let socket = socket2::Socket::new(
            socket2::Domain::from(libc::AF_CAN),
            socket2::Type::RAW,
            Some(socket2::Protocol::from(libc::CAN_RAW))
        )?;

        socket.set_nonblocking(true)?;

        socket.bind(&Self::interface_name_to_socket_addr(interface))?;

        // convert interface name to a SockAddr
        // raw_socket.bind(Self::interface_name_to_socket_addr(interface));


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

        let filters = &self.handlers.keys()
            .map(|header| {
                libc::can_filter {
                    can_id: header.inner as u32,
                    can_mask: 0xFFFFFFFF_u32
                }
            }).collect::<Vec<libc::can_filter>>();



        // copied from the C++ implementation
        // TODO: Find a safer way to set filters
        // TODO: work out if filters are even needed (maybe filter for only payload bus?)
        unsafe {
            libc::setsockopt(
                self.inner.as_raw_fd(),
                libc::SOL_CAN_RAW,
                libc::CAN_RAW_FILTER,
                filters.as_ptr() as *const libc::c_void,
                (filters.len() * size_of::<libc::can_filter>()) as u32
            );
        }

        // add header to the list of CAN id filters
        // self.inner.attach_filter(filters)

        // self.inner.set_filters(
        //     &self.handlers.keys()
        //     .map(|header| {
        //         CanFilter::new(header.inner as u32, 0xFFFFFFFF_u32)
        //     }).collect::<Vec<CanFilter>>()
        // )?;
        Ok(())
    }



    /// Will try to transmit non-blocking. If that fails, try again in blocking mode
    pub fn send<P>(&mut self, group_id: GroupID, device_id: DeviceID, packet: &P) -> Result<(), std::io::Error> where P: OutboundCANPacketV3 {
        let header = CANHeader::new(group_id, device_id, P::COMMAND_ID);
        let data = packet.encode();

        let frame = CANFrame::new(header, data);

        return self.inner.write_all(unsafe { frame.as_bytes() });
    }



    /// If `Ok(true)`, we successfully received and handled a message
    /// 
    /// If `Ok(false)`, we did not receive a message
    pub fn read_msg(&mut self) -> Result<bool, std::io::Error> {
        let mut frame = CANFrame::zeroed();

        match self.inner.read_exact(unsafe { frame.as_bytes_mut() }) {
            // successfully read frame
            Ok(()) => {
                let header = CANHeader::try_from(frame.inner.can_id)?;
        
                match self.handlers.get(&header) {
                    Some(CANPacketHandler::Ping(f)) => f(header),
                    Some(CANPacketHandler::BrInt8(f)) => f(header, *data_to_i8(&frame.inner.data)?),
                    Some(CANPacketHandler::BrInt16(f)) => f(header, *data_to_i16(&frame.inner.data)?),
                    Some(CANPacketHandler::BrFloat(f)) => f(header, *data_to_f32(&frame.inner.data)?),
                    Some(CANPacketHandler::BrData(f)) => f(header, frame.inner.data),
                    Some(CANPacketHandler::BrData2(f)) => f(header, frame.inner.data),
                    _ => {}
                }

                return Ok(true);
            },
            // if there was unexpected eof, then there was no frame to read, so return Ok(false),
            // other errors return usual Err(e)
            Err(err) => {
                match err.kind() {
                    std::io::ErrorKind::UnexpectedEof => { return Ok(false); },
                    _ => { return Err(err); }
                }
            }
        }
    }
}

