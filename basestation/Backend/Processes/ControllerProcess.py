# An example process
from AbstractProcess import AbstractProcess
from DEFCOM import ChannelLossyCast
import websocket
import json
import time


"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""

class ControllerProcess(AbstractProcess):
    def handleWSMessage(self, ws, message):
        # Get the message
        # We expect the message to be a json of the form:
        # {"LeftSpeed": 0, "RightSpeed": 0}
        decoded = json.loads(message)
        print("Message: ", decoded)

        message = self.channel.getNewMessageObject()
        message.setFloat("left_stick", decoded["LeftSpeed"])
        message.setFloat("right_stick", decoded["RightSpeed"])

    def __init__(self) -> None:
        # Start a DEFCOM Channel to send to rover
        self.channel = ChannelLossyCast.ChannelLossyCast("COMFILES/DriveController.defcom")

        # Create websocket to listen for xbox controller
        self.ws = websocket.WebSocketApp(
            "ws://localhost:8765",
            on_message=self.handleWSMessage,
        )

        
        
        
    def run(self) -> None:
        self.ws.run_forever()
