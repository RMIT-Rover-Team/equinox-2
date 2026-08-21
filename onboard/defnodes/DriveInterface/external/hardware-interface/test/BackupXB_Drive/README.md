# ODriveEmulator
A SocketCAN Emulator for ODrive hardware

Allows a user to use a socketcan device to emulate the basic functionality of an ODrive.

This library implements all basic Velocity / Position / Torque / Sensing in the ODrive command set.

All changes are instantaneous.

Can interface must be configured beforehand in the following manner:

`sudo ip link set <Interface> type can bitrate <Baudrate>`

`sudo ip link set dev <Interface> up`


EG:

`sudo ip link set can0 type can bitrate 1000000`

`sudo ip link set dev can0 up`