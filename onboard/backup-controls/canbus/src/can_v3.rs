

use std::ops::Range;

use intbits::Bits;


#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CANHeader {
    pub inner: u16,
}

impl CANHeader {
    const COMMAND_ID_BITS: Range<u8> = 0..4;
    const DEVICE_ID_BITS: Range<u8> = 4..9;
    const GROUP_ID_BITS: Range<u8> = 9..11;

    pub fn new(group_id: GroupID, device_id: DeviceID, command_id: CommandID) -> Self {
        // the errors below should never happen
        let mut inner: u16 = 0;
        inner.set_bits(Self::GROUP_ID_BITS, group_id.discriminant() as u16);
        inner.set_bits(Self::DEVICE_ID_BITS, device_id.inner as u16);
        inner.set_bits(Self::COMMAND_ID_BITS, command_id.discriminant() as u16);
        
        return Self {
            inner: inner
        };
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

impl TryFrom<u16> for CANHeader {
    type Error = std::io::Error;
    fn try_from(value: u16) -> Result<Self, Self::Error> {
        return match value.bit_width() {
            0..=11 => Ok(Self { inner: value }),
            _ => Err(std::io::Error::other("CAN header does not fit into 11 bits"))
        };
    }
}

impl TryFrom<u32> for CANHeader {
    type Error = std::io::Error;
    fn try_from(value: u32) -> Result<Self, Self::Error> {
        return match value.bit_width() {
            0..=11 => Ok(Self { inner: value as u16 }),
            _ => Err(std::io::Error::other("CAN header does not fit into 11 bits"))
        };
    }
}


#[repr(u8)]
#[derive(Debug)]
pub enum GroupID {
    Onboard = 0x0,
    ReservedForMyActuator = 0x1,
    Payload = 0x2,
    Unused = 0x3
}

impl GroupID {
    pub fn discriminant(&self) -> u8 {
        // SAFETY: Because `Self` is marked `repr(u8)`, its layout is a `repr(C)` `union`
        // between `repr(C)` structs, each of which has the `u8` discriminant as its first
        // field, so we can read the discriminant without offsetting the pointer.
        unsafe { *<*const _>::from(self).cast::<u8>() }
    }

    pub fn from_u16(x: u16) -> Option<Self> {
        match x {
            0x0 => Some(Self::Onboard),
            0x1 => Some(Self::ReservedForMyActuator),
            0x2 => Some(Self::Payload),
            0x3 => Some(Self::Unused),
            _ => None
        }
    }
}

// 5 bit int
pub struct DeviceID {
    inner: u8
}

impl TryFrom<u8> for DeviceID {
    type Error = std::io::Error;
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        if value.bit_width() <= 5 { return Ok(DeviceID { inner: value }); }
        else { return Err(std::io::Error::other("device id too large (>5 bits)")); }
    }
}

impl From<DeviceID> for u8 {
    fn from(value: DeviceID) -> Self {
        return value.inner;
    }
}


#[repr(u8)]
#[derive(Debug, PartialEq, Eq)]
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

impl CommandID {
    pub fn discriminant(&self) -> u8 {
        // SAFETY: Because `Self` is marked `repr(u8)`, its layout is a `repr(C)` `union`
        // between `repr(C)` structs, each of which has the `u8` discriminant as its first
        // field, so we can read the discriminant without offsetting the pointer.
        unsafe { *<*const _>::from(self).cast::<u8>() }
    }

    pub fn from_u16(x: u16) -> Option<Self> {
        match x {
            0x0 => Some(Self::Ping),

            0x1 => Some(Self::EStop),
            0x2 => Some(Self::TxInt8),
            0x3 => Some(Self::TxInt16),
            0x4 => Some(Self::TxFloat),
            0x5 => Some(Self::TxData),
            0x6 => Some(Self::TxData2),

            0xA => Some(Self::TxInt8),
            0xB => Some(Self::TxInt16),
            0xC => Some(Self::TxFloat),
            0xD => Some(Self::TxData),
            0xE => Some(Self::TxData2),
            _ => None
        }
    }
}




pub enum SendCANCommand {
    Ping,
    EStop,
    TxInt8([i8; 8]),
    TxInt16([i16; 4]),
    TxFloat([f32; 2]),
    TxData([u8; 8]),
    TxData2([u8; 8]),
}




#[repr(u8)]
#[derive(Debug, Clone)]
pub enum CANPacketHandler {
    Ping(fn(CANHeader)) = 0,
    BrInt8(fn(CANHeader, [i8; 8])) = 1,
    BrInt16(fn(CANHeader, [i16; 4])) = 2,
    BrFloat(fn(CANHeader, [f32; 2])) = 3,
    BrData(fn(CANHeader, [u8; 8])) = 4,
    BrData2(fn(CANHeader, [u8; 8])) = 5
}

impl CANPacketHandler {
    pub fn command_id(&self) -> CommandID {
        return match self {
            CANPacketHandler::Ping(_) => CommandID::Ping,
            CANPacketHandler::BrInt8(_) => CommandID::BrInt8,
            CANPacketHandler::BrInt16(_) => CommandID::BrInt16,
            CANPacketHandler::BrFloat(_) => CommandID::BrFloat,
            CANPacketHandler::BrData(_) => CommandID::BrData,
            CANPacketHandler::BrData2(_) => CommandID::BrData2
        };
    }

    pub fn discriminant(&self) -> u8 {
        // SAFETY: Because `Self` is marked `repr(u8)`, its layout is a `repr(C)` `union`
        // between `repr(C)` structs, each of which has the `u8` discriminant as its first
        // field, so we can read the discriminant without offsetting the pointer.
        unsafe { *<*const _>::from(self).cast::<u8>() }
    }
}



pub fn data_to_i8(data: &[u8; 8]) -> Result<&[i8; 8], std::io::Error> {
    let (_prefix, converted, _suffix) = unsafe { data.align_to::<i8>() };

    return converted.as_array().ok_or(std::io::Error::other("converted data was not 8 bytes, original can_frame was not aligned to 8 bytes"));
}

pub fn data_to_i16(data: &[u8; 8]) -> Result<&[i16; 4], std::io::Error> {
    let (_prefix, converted, _suffix) = unsafe { data.align_to::<i16>() };

    return converted.as_array().ok_or(std::io::Error::other("converted data was not 8 bytes, original can_frame was not aligned to 8 bytes"));
}

pub fn data_to_f32(data: &[u8; 8]) -> Result<&[f32; 2], std::io::Error> {
    let (_prefix, converted, _suffix) = unsafe { data.align_to::<f32>() };

    return converted.as_array().ok_or(std::io::Error::other("converted data was not 8 bytes, original can_frame was not aligned to 8 bytes"));
}
