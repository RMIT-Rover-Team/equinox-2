#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast
from DEFCOM import ChannelLossyCast

import board
import adafruit_dotstar as dotstar

# clock first, data second: D4 = pin 7, D22 = pin 15
class statusLampHandler:
    def __init__(self):
        # Init a DEFCOM Channel
        self.channel = ChannelLossyCast.LossyCastSubscriber("COMFILES/StatusIndicator.defcom")
        
        self.pixels = dotstar.DotStar(board.D4, board.D22, 10, brightness=0.2, auto_write=True)
        self.pixels.fill((255, 255, 255)) # SAFE

    def run(self):
        while True:
            message = self.channel.subscribe()

            self.pixels.fill((message.getChar('R')[0], message.getChar('G')[0], message.getChar('B')[0]))

       



if __name__ == "__main__":
    app = statusLampHandler()
    app.run()