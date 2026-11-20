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




class ExcavatorProcess(AbstractProcess):
    def __init__(self,lostPayloadTime=20) -> None:
        print("Starting Excavator")

        # Start the listener for Excavator
        self.telemChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/ExcavatorStatus.defcom")
        self.controllerChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/XBox.defcom")
        self.excavatorChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/Excavator.defcom")
        print("Established Excavator Channel")

        # Start the websocket stub for this process
        self.globalProcessDict['webserver'].newHandler("/ws/excavatorStatus", self._wshandler)

        # Returned Telem Values
        self.FramePosition = 0
        self.BucketPosition = 0
        self.TeethPosition = 0
        self.Magnet = 0
        self.PayloadWorking = 0

        # Drive State Outgoing
        self.SelectedMagnet = togglerButton(0)
        self.SelectedESTOP = togglerButton(0)

        # Websocket Handler Signalling to ensure that all clients get alerted when message arrives
        self.waitCondition = threading.Condition()
        self.latestID = 0

        # The amount of time before we consider the payload to have been lost
        self.lostPayloadTime = lostPayloadTime


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
                        "FramePosition": self.FramePosition,
                        "BucketPosition": self.BucketPosition,
                        "TeethPosition": self.TeethPosition,
                        "Magnet": self.Magnet,
                        "PayloadWorking": self.PayloadWorking
                    }
                ))
            except Exception:
                break
        Connection.close()

    # The Telem Channel Listener
    def _telemListener(self):
        lastLastSeenTime = 0
        while True:
            msg = self.telemChannel.subscribe()

            self.FramePosition = msg.getFloat("FramePosition")
            self.BucketPosition = msg.getFloat("BucketPosition")
            self.TeethPosition = msg.getFloat("TeethPosition")
            self.Magnet = msg.getChar("Magnet")[0]

            lastSeenTime = msg.getLong("PayloadLastSeen")
            if (lastSeenTime - lastLastSeenTime) > self.lostPayloadTime:
                self.PayloadWorking = 0
            else:
                self.PayloadWorking = 1
            
            lastLastSeenTime = lastSeenTime

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
            outgoing = self.excavatorChannel.getNewMessageObject()

            # Make sure it is excavator which is channel 3, otherwise we ignore the message
            if msg.getInt("Channel_ID") != 3:
                continue

            # Get the Stick Values
            frameSpeed = msg.getFloat("L_Stick_Y") * 1
            bucketSpeed = msg.getFloat("R_Stick_Y") * 1
            outgoing.setFloat("FrameSpeed", frameSpeed)
            outgoing.setFloat("BucketSpeed", bucketSpeed)

            # If the DPAD Up is pressed, we Teeth Out
            if msg.getChar("D_Pad_Up")[0] == 1:
                outgoing.setFloat("TeethSpeed", 1)
            # DPAD Down = Teeth In
            elif msg.getChar("D_Pad_Down")[0] == 1:
                outgoing.setFloat("TeethSpeed", -1)
            # Otherwise Stop
            else:
                outgoing.setFloat("TeethSpeed", 0)

            
            # If Y is pressed, we switch magnet
            self.SelectedMagnet.update(msg.getChar("Y_Button")[0])
            outgoing.setChar("Magnet", self.SelectedMagnet.getValue().to_bytes(1))
            
            # If X is pressed, we ESTOP
            self.SelectedESTOP.update(msg.getChar("X_Button"))
            outgoing.setChar("EStop", self.SelectedESTOP.getValue().to_bytes(1))

            # Send the message
            self.excavatorChannel.publish(outgoing)





        
