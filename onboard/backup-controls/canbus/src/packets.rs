use crate::can_v3::CommandID;

pub trait CANPacketV3 {
    const COMMAND_ID: CommandID;

}
pub trait OutboundCANPacketV3: CANPacketV3 {
    fn encode(&self) -> [u8; 8];
}
pub trait InboundCANPacketV3: CANPacketV3 {
    fn decode(bytes: [u8; 8]) -> Self;
}





pub struct CANV3Ping;
pub struct CANV3EStop;
pub struct CANV3TxInt8([i8; 8]);
pub struct CANV3TxInt16([i16; 4]);
pub struct CANV3TxFloat([f32; 2]);
pub struct CANV3TxData([u8; 8]);
pub struct CANV3TxData2([u8; 8]);
pub struct CANV3BrInt8([i8; 8]);
pub struct CANV3BrInt16([i16; 4]);
pub struct CANV3BrFloat([f32; 2]);
pub struct CANV3BrData([u8; 8]);
pub struct CANV3BrData2([u8; 8]);


impl CANPacketV3 for CANV3Ping { const COMMAND_ID: CommandID = CommandID::Ping; }
impl CANPacketV3 for CANV3EStop { const COMMAND_ID: CommandID = CommandID::EStop; }
impl CANPacketV3 for CANV3TxInt8 { const COMMAND_ID: CommandID = CommandID::TxInt8; }
impl CANPacketV3 for CANV3TxInt16 { const COMMAND_ID: CommandID = CommandID::TxInt16; }
impl CANPacketV3 for CANV3TxFloat { const COMMAND_ID: CommandID = CommandID::TxFloat; }
impl CANPacketV3 for CANV3TxData { const COMMAND_ID: CommandID = CommandID::TxData; }
impl CANPacketV3 for CANV3TxData2 { const COMMAND_ID: CommandID = CommandID::TxData2; }
impl CANPacketV3 for CANV3BrInt8 { const COMMAND_ID: CommandID = CommandID::BrInt8; }
impl CANPacketV3 for CANV3BrInt16 { const COMMAND_ID: CommandID = CommandID::BrInt16; }
impl CANPacketV3 for CANV3BrFloat { const COMMAND_ID: CommandID = CommandID::BrFloat; }
impl CANPacketV3 for CANV3BrData { const COMMAND_ID: CommandID = CommandID::BrData; }
impl CANPacketV3 for CANV3BrData2 { const COMMAND_ID: CommandID = CommandID::BrData2; }


impl OutboundCANPacketV3 for CANV3Ping {
    fn encode(&self) -> [u8; 8] {
        return [0; 8];
    }
}
impl OutboundCANPacketV3 for CANV3EStop {
    fn encode(&self) -> [u8; 8] {
        return [0; 8];
    }
}

macro_rules! impl_outbound_packet {
    ($ty:ty) => {
        impl OutboundCANPacketV3 for $ty {
            fn encode(&self) -> [u8; 8] {
                return unsafe {
                    std::mem::transmute::<_, [u8; 8]>(self.0)
                };
            }
        }
    };
}

impl_outbound_packet!(CANV3TxInt8);
impl_outbound_packet!(CANV3TxInt16);
impl_outbound_packet!(CANV3TxFloat);
impl_outbound_packet!(CANV3TxData);
impl_outbound_packet!(CANV3TxData2);






impl InboundCANPacketV3 for CANV3Ping {
    fn decode(_bytes: [u8; 8]) -> Self {
        return Self {  };
    }
}

macro_rules! impl_inbound_packet {
    ($ty:ty) => {
        impl InboundCANPacketV3 for $ty {
            fn decode(bytes: [u8; 8]) -> Self {
                return unsafe {
                    Self(std::mem::transmute::<[u8; 8], _>(bytes))
                };
            }
        }
    };
}

impl_inbound_packet!(CANV3BrInt8);
impl_inbound_packet!(CANV3BrInt16);
impl_inbound_packet!(CANV3BrFloat);
impl_inbound_packet!(CANV3BrData);
impl_inbound_packet!(CANV3BrData2);








