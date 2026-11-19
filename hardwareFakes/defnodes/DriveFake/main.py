#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast
from DEFCOM import ChannelLossyCast
import threading
import time

class DriveFake():
    def __init__(self):
        self.LeftPosition = 0
        self.RightPosition = 0

        self.LeftSpeed = 0
        self.RightSpeed = 0

        self.Enabled = 1
        self.SlipAlarm = 0

        self.DriveMode = 0

        # Set up the Channels
        self.inputChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/Drive.defcom")
        self.outputChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/DriveStatus.defcom")

        # Start the telem thread
        self.telemThread = threading.Thread(target=self._telemThread, args=())
        self.telemThread.start()


    def _telemThread(self):
        while True:
            msg = self.outputChannel.getNewMessageObject()
            msg.setFloat("LeftSpeed",self.LeftSpeed)
            msg.setFloat("RightSpeed",self.RightSpeed)
            msg.setFloat("LeftPosition",self.LeftPosition)
            msg.setFloat("RightPosition",self.RightPosition)
            msg.setChar("DriveMode",self.DriveMode.to_bytes(1))
            msg.setChar("Enabled",self.Enabled.to_bytes(1))
            msg.setChar("SlipAlarm",self.SlipAlarm.to_bytes(1))
            self.outputChannel.publish(msg)

            time.sleep(0.5)

            # Calculate displacement
            self.LeftPosition += self.LeftSpeed * 0.5
            self.RightPosition += self.RightSpeed * 0.5

    def run(self):
        while True:
            msg = self.inputChannel.subscribe()

            # Get the Speeds
            self.LeftSpeed = msg.getFloat("LeftSpeed")
            self.RightSpeed = msg.getFloat("RightSpeed")

            # Get the drive mode
            self.DriveMode = msg.getChar("DriveMode")[0]

            # Get the enable state
            self.Enabled = msg.getChar("Enable")[0]




if __name__ == "__main__":
    app = DriveFake()
    app.run()