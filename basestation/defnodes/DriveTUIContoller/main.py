#!/usr/bin/env python3
import curses
from DEFCOM import ChannelLossyCast

def main(stdscr):
    publisher = ChannelLossyCast.LossyCastPublisher("COMFILES/DriveController.defcom")

    leftSpeed = 0.0
    rightSpeed = 0.0

    curses.cbreak()
    stdscr.nodelay(True)
    stdscr.keypad(True)

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

        # Publish updated speeds
        message= publisher.getNewMessageObject()
        message.setFloat("left_stick", leftSpeed)
        message.setFloat("right_stick", rightSpeed)
        publisher.publish(message)
        
        

        # Display current values
        stdscr.clear()
        stdscr.addstr(0, 0, f"Left: {leftSpeed:.2f}")
        stdscr.addstr(1, 0, f"Right: {rightSpeed:.2f}")
        stdscr.addstr(3, 0, "Use arrow keys. Press 'q' to quit.")
        stdscr.refresh()

        curses.napms(50)

if __name__ == "__main__":
    curses.wrapper(main)
