#!/usr/bin/env python3

import torque
import StatusIndicator as SI
import TCPstreams5 as tcp
import struct

if __name__ == '__main__':
    server = tcp.newServer("0.0.0.0",9000)

    client = tcp.serverCon(server)
    print("Accept Client")

    myCon = torque.TorqueHandler("can0")
    myCon.set_mode(torque.UNLOCKED_VELOCITY)
    myCon.enable()

    SI.setLED(SI.LEDCOLOUR.SAFE)
    while True:
        d = client.getdat()

        Left, Right = struct.unpack("<dd",d)
        print("Speed",Left,Right)
        myCon.set_speed(Left, Right)

        if (Left != 0) and (Right != 0):
            SI.setLED(SI.LEDCOLOUR.AUTO)
        else:
            SI.setLED(SI.LEDCOLOUR.AUTO_PREP)