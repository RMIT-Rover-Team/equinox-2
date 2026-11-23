
use crate::can_v3::{CANHeader, CommandID};






// pub enum CANPacket {
//     Ping { header: CANHeader },
//     EStop { header: CANHeader },

//     TxInt8 { header: CANHeader, data: [i8; 8] },
//     TxInt16 { header: CANHeader, data: [i16; 4] },
//     TxFloat { header: CANHeader, data: [f32; 2] },
//     TxData { header: CANHeader, data: [u8; 8] },
//     TxData2 { header: CANHeader, data: [u8; 8] },

//     BrInt8 { header: CANHeader, data: [i8; 8] },
//     BrInt16 { header: CANHeader, data: [i16; 4] },
//     BrFloat { header: CANHeader, data: [f32; 2] },
//     BrData { header: CANHeader, data: [u8; 8] },
//     BrData2 { header: CANHeader, data: [u8; 8] },
// }


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
