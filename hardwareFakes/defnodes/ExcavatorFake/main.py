#!/usr/bin/env python3
import threading
import time

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast
from DEFCOM import ChannelLossyCast

class fakeJoint:
    def __init__(self):
        self._p = 0
        self._v = 0
        self.last = time.monotonic()

    def set_velocity(self,v):
        self._v = v

    def tick(self):
        t = time.monotonic()
        self._p += self._v * (t - self.last)
        self.last = t

    def get_position(self):
        return self._p



class ExcavatorFake:
    def __init__(self):
        self.excavator_tilt = fakeJoint()
        self.excavator_bucket = fakeJoint()
        self.excavator_teeth = fakeJoint()

        self.excavator_magnet = 0

        # Create the DEFCOM Channels
        self.inputChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/Excavator.defcom")
        self.outputChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/ExcavatorStatus.defcom")

    def _telemThread(self):
        while True:
            time.sleep(0.5)

            self.excavator_bucket.tick()
            self.excavator_tilt.tick()
            self.excavator_teeth.tick()

            # Send a status message
            outgoing = self.outputChannel.getNewMessageObject()
            outgoing.setFloat('FramePosition', self.excavator_bucket.get_position())
            outgoing.setFloat('BucketPosition', self.excavator_tilt.get_position())
            outgoing.setFloat('TeethPosition', self.excavator_teeth.get_position())
            outgoing.setChar('Magnet', self.excavator_magnet.to_bytes(1))
            outgoing.setLong('PayloadLastSeen', int(time.monotonic()))

            self.outputChannel.publish(outgoing)

            print("Current Position: ", self.excavator_bucket.get_position(), self.excavator_tilt.get_position(), self.excavator_teeth.get_position())
            print("Magnet State: ", self.excavator_magnet)

    def run(self):
        t = threading.Thread(target=self._telemThread)
        t.start()

        while True:
            msg = self.inputChannel.subscribe()

            # Check for ESTOP
            if msg.getChar('EStop')[0]:
                # Halt all movement
                self.excavator_bucket.set_velocity(0)
                self.excavator_tilt.set_velocity(0)
                self.excavator_teeth.set_velocity(0)
                self.excavator_magnet = 0

            # Get the SetSpeeds
            else:
                self.excavator_bucket.set_velocity(msg.getFloat('BucketSpeed'))
                self.excavator_tilt.set_velocity(msg.getFloat('FrameSpeed'))
                self.excavator_teeth.set_velocity(msg.getFloat('TeethSpeed'))
                self.excavator_magnet = msg.getChar('Magnet')[0]


if __name__ == "__main__":
    app = ExcavatorFake()
    app.run()