

use std::ops::Range;

use intbits::Bits;
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::{FromPrimitive, ToPrimitive};



pub struct CANHeader {
    pub inner: u16
}

impl CANHeader {
    const COMMAND_ID_BITS: Range<u8> = 0..4;
    const DEVICE_ID_BITS: Range<u8> = 4..9;
    const GROUP_ID_BITS: Range<u8> = 9..11;

    pub fn new(group_id: GroupID, device_id: u8, command_id: CommandID) -> Result<Self, socketcan::Error> {
        if device_id > 2_u8.pow(5) {
            return Err(socketcan::Error::Io(std::io::Error::other("failed to create CAN header, device id too large")));
        }

        // the errors below should never happen
        let mut inner: u16= 0;
        inner.set_bits(Self::GROUP_ID_BITS, GroupID::to_u16(&group_id)
            .ok_or(socketcan::Error::Io(std::io::Error::other("failed to create CAN header, failed to parse group id")))?);
        inner.set_bits(Self::DEVICE_ID_BITS, device_id as u16);
        inner.set_bits(Self::COMMAND_ID_BITS, CommandID::to_u16(&command_id)
            .ok_or(socketcan::Error::Io(std::io::Error::other("failed to create CAN header, failed to parse command id")))?);
        
        return Ok(Self { inner: inner });
    }

    pub fn get_group_id(&self) -> GroupID {
        return GroupID::from_u16(self.inner.bits(Self::GROUP_ID_BITS)).unwrap();
    }

    pub fn get_device_id(&self) -> u8 {
        return self.inner.bits(Self::DEVICE_ID_BITS) as u8;
    }

    pub fn get_command_id(&self) -> CommandID {
        return CommandID::from_u16(self.inner.bits(Self::COMMAND_ID_BITS)).unwrap();
    }
}


#[repr(u8)]
#[derive(Debug, FromPrimitive, ToPrimitive)]
pub enum GroupID {
    Onboard = 0x0,
    ReservedForMyActuator = 0x1,
    Payload = 0x2,
    Unused = 0x3
}


#[repr(u8)]
#[derive(Debug, FromPrimitive, ToPrimitive)]
pub enum CommandID {
    Ping = 0x0,

    // master -> slave
    EStop = 0x1,
    TxInt8 = 0x2,
    TxInt16 = 0x3,
    TxFloat = 0x4,
    TxData = 0x5,
    TxData2 = 0x6,

    // slave -> master
    BrInt8 = 0xA,
    BrInt16 = 0xB,
    BrFloat = 0xC,
    BrData = 0xD,
    BrData2 = 0xE,
}