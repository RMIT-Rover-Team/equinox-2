# An example process
from AbstractProcess import AbstractProcess
from DEFCOM import ChannelLossyCast
import time
import WSPageStub
import json
import threading
from Processes.common import togglerButton

"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""




class DriveProcess(AbstractProcess):
    def __init__(self,maxDriveSpeed=100) -> None:
        print("Starting Drive")

        # Start the listener for drive
        self.telemChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/DriveStatus.defcom")
        self.controllerChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/XBox.defcom")
        self.driveChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/Drive.defcom")
        print("Established Drive Channel")

        # Start the websocket stub for this process
        self.globalProcessDict['webserver'].newHandler("/ws/driveStatus", self._wshandler)

        # Returned Telem Values
        self.leftSpeed = 0
        self.rightSpeed = 0
        self.leftPosition = 0
        self.rightPosition = 0
        self.DriveMode = 0
        self.Enabled = 0
        self.Slip = 0

        # Drive State Outgoing
        self.SelectedDriveMode = togglerButton(0)
        self.SelectedEnable = togglerButton(1)

        # Websocket Handler Signalling to ensure that all clients get alerted when message arrives
        self.waitCondition = threading.Condition()
        self.latestID = 0

        self.maxDriveSpeed = maxDriveSpeed


    # Will spawn asynchronously
    def _wshandler(self, Connection: WSPageStub.WSConnection):
        lastSeenID = 0
        while True:
            # Wait for the condition
            with self.waitCondition:
                self.waitCondition.wait_for(lambda: self.latestID > lastSeenID) # Threads wait here until the main thread tells us to
                self.latestID = lastSeenID

            # Send a message
            try:
                Connection.send(json.dumps(
                    {
                        "LeftSpeed":self.leftSpeed,
                        "RightSpeed":self.rightSpeed,
                        "LeftPosition": self.leftPosition,
                        "RightPosition": self.rightPosition,
                        "DriveMode":self.DriveMode,
                        "Enabled":self.Enabled,
                        "Slip":self.Slip
                    }
                ))
            except Exception:
                break
        Connection.close()

    # The Telem Channel Listener
    def _telemListener(self):
        while True:
            msg = self.telemChannel.subscribe()

            self.leftSpeed = msg.getFloat("LeftSpeed")
            self.rightSpeed = msg.getFloat("RightSpeed")
            self.leftPosition = msg.getFloat("LeftPosition")
            self.rightPosition = msg.getFloat("RightPosition")
            self.DriveMode = msg.getChar("DriveMode")[0]
            self.Enabled = msg.getChar("Enabled")[0]
            self.Slip = msg.getChar("SlipAlarm")[0]

            #print("MSG ID = ",self.latestID)

            # Update the condition and let connected devices know
            with self.waitCondition:
                self.latestID += 1
                self.waitCondition.notify_all()


    def run(self) -> None:
        self._telemThread = threading.Thread(target=self._telemListener, daemon=True)
        self._telemThread.start()

        # Handle the XBox Control of the Drive Stack

        while True:
            # Get Message
            msg = self.controllerChannel.subscribe()
            # Start Outgoing Message
            outgoing = self.driveChannel.getNewMessageObject()

            # Make sure it is drive which is channel 1, otherwise we ignore the message
            if msg.getInt("Channel_ID") != 1:
                continue

            # Get the Stick Values
            leftSpeed = msg.getFloat("L_Stick_Y") * self.maxDriveSpeed
            rightSpeed = msg.getFloat("R_Stick_Y") * self.maxDriveSpeed
            outgoing.setFloat("LeftSpeed", leftSpeed)
            outgoing.setFloat("RightSpeed", rightSpeed)

            # If Y is pressed, we switch drive Mode
            self.SelectedDriveMode.update(msg.getChar("Y_Button")[0])
            if self.SelectedDriveMode.getValue() == 1:
                outgoing.setChar("DriveMode", b'\x02')
            else:
                outgoing.setChar("DriveMode", b'\x00')


            # If A is pressed, we toggle Enable
            self.SelectedEnable.update(msg.getChar("A_Button")[0])
            outgoing.setChar("Enable", self.SelectedEnable.getValue().to_bytes(1))
            

            # If X is pressed, we ESTOP
            outgoing.setChar("EStop", msg.getChar("X_Button"))

            # Send the message
            self.driveChannel.publish(outgoing)





        
