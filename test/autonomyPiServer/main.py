#!/usr/bin/env python3

import torque
import TCPstreams5 as tcp
import struct

if __name__ == '__main__':
    server = tcp.newServer("0.0.0.0",9000)

    client = tcp.serverCon(server)
    print("Accept Client")

    myCon = torque.TorqueHandler("vcan0")
    myCon.set_mode(torque.UNLOCKED_VELOCITY)
    myCon.enable()


    while True:
        d = client.getdat()

        Left, Right = struct.unpack("<dd",d)
        print("Speed",Left,Right)
        myCon.set_speed(Left, Right)