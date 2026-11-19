# An example process
from AbstractProcess import AbstractProcess
from DEFCOM import ChannelLossyCast
import time
import WSPageStub
import json
import threading


"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""

class DriveProcess(AbstractProcess):
    def __init__(self) -> None:
        print("Starting Drive")

        # Start the listener for drive
        self.telemChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/DriveStatus.defcom")
        print("Established Drive Channel")

        # Start the websocket stub for this process
        self.globalProcessDict['webserver'].newHandler("/ws/driveStatus", self._wshandler)


        self.leftSpeed = 0
        self.rightSpeed = 0
        self.leftPosition = 0
        self.rightPosition = 0
        self.DriveMode = 0
        self.Enable = 0
        self.Slip = 0

        # Websocket Handler Signalling to ensure that all clients get alerted when message arrives
        self.waitCondition = threading.Condition()
        self.latestID = 0


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
                        "Enabled":self.Enable,
                        "Slip":self.Slip
                    }
                ))
            except Exception:
                break
        Connection.close()






            
        
    def run(self) -> None:
        while True:
            msg = self.telemChannel.subscribe()

            self.leftSpeed = msg.getFloat("LeftSpeed")
            self.rightSpeed = msg.getFloat("RightSpeed")
            self.leftPosition = msg.getFloat("LeftPosition")
            self.rightPosition = msg.getFloat("RightPosition")
            self.DriveMode = msg.getChar("DriveMode")[0]
            self.Enable = msg.getChar("Enabled")[0]
            self.Slip = msg.getChar("SlipAlarm")[0]

            #print("MSG ID = ",self.latestID)

            # Update the condition and let connected devices know
            with self.waitCondition:
                self.latestID += 1
                self.waitCondition.notify_all()
