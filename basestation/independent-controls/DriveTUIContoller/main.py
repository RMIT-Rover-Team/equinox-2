#!/usr/bin/env python3
import curses
from DEFCOM import ChannelLossyCast

def main(stdscr):
    publisher = ChannelLossyCast.LossyCastPublisher("COMFILES/Drive.defcom")

    leftSpeed = 0.0
    rightSpeed = 0.0

    curses.cbreak()
    stdscr.nodelay(True)
    stdscr.keypad(True)

    Mode = 0
    Enable = 1
    EStop = 0
    Calibrate = 0
    while True:
        key = stdscr.getch()

        if key == curses.KEY_UP:
            leftSpeed += 5
            rightSpeed += 5

        elif key == curses.KEY_DOWN:
            leftSpeed -= 5
            rightSpeed -= 5

        elif key == curses.KEY_LEFT:
            leftSpeed -= 2.5
            rightSpeed += 2.5

        elif key == curses.KEY_RIGHT:
            leftSpeed += 2.5
            rightSpeed -= 2.5

        elif key == ord('q'):
            break

        elif key == ord(' '):
            leftSpeed = 0
            rightSpeed = 0

        elif key == ord('e'):
            EStop = 1

        elif key == ord('o'):
            if Enable == 1:
                Enable = 0
            else:
                Enable = 1

        elif key == ord('c'):
            Calibrate = 1

        elif key == ord('t'):
            Mode = 2

        elif key == ord('u'):
            Mode = 0

        # Publish updated speeds
        message= publisher.getNewMessageObject()
        message.setFloat("LeftSpeed", leftSpeed)
        message.setFloat("RightSpeed", rightSpeed)
        message.setChar("DriveMode", Mode.to_bytes(1))
        message.setChar("Enable", Enable.to_bytes(1))
        message.setChar("EStop", EStop.to_bytes(1))
        message.setChar("Calibrate", Calibrate.to_bytes(1))
        publisher.publish(message)

        EStop = 0
        Calibrate = 0
        
        

        # Display current values
        stdscr.clear()
        stdscr.addstr(0, 0, f"Left: {leftSpeed:.2f}")
        stdscr.addstr(1, 0, f"Right: {rightSpeed:.2f}")
        stdscr.addstr(2, 0, f"Mode: {Mode}")
        stdscr.addstr(3, 0, "Enable: " + str(Enable))
        stdscr.addstr(4, 0, "EStop: " + str(EStop))
        stdscr.addstr(5, 0, "Calibrate: " + str(Calibrate))
        stdscr.addstr(6, 0, "Use arrow keys. Press 'q' to quit.")
        stdscr.refresh()

        curses.napms(50)

if __name__ == "__main__":
    curses.wrapper(main)
