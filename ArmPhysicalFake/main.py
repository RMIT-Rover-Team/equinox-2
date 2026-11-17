#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast
from DEFCOM import ChannelLossyCast
import time
import threading

class FakeArm:
    def __init__(self):
        self.jointPositions = [0, 0, 0, 0, 0, 0]
        self.targetJointPositions = [0, 0, 0, 0, 0, 0]
        self.jointSpeeds = [0, 0, 0, 0, 0, 0]
        self.idleMoveSpeedDiv = 0.5
        self.td = 1/6

        # Set up the defcom channels for input and output
        self.outputChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/ArmState.defcom")
        self.inputChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/ArmControl.defcom")

        # Start the sender thread
        self.stateThread = threading.Thread(target=self._stateThread)
        self.stateThread.start()

        print("Fake Arm Ready!")

    def _stateThread(self):
        while True:
            # Send a message to the output channel
            print("Joint Positions: ",self.jointPositions, "Joint Speeds: ", self.jointSpeeds)
            message = self.outputChannel.getNewMessageObject()

            for i in range(6):
                message.setFloat("jointPositions",self.jointPositions[i],i)

            self.outputChannel.publish(message)

            time.sleep(self.td)

            # Idle Movement
            # Check velocities
            velMode = False
            for i in range(6):
                if self.jointSpeeds[i] != 0:
                    velMode = True
                self.jointPositions[i] += self.jointSpeeds[i] * self.td
                self.targetJointPositions[i] = self.jointPositions[i] # Update so it doesn't complain

            # if it was position instead, then we just idle move instead
            if velMode == False:
                self.jointPositions = self.targetJointPositions
                for i in range(6):
                    self.jointPositions[i] += self.jointSpeeds[i] * self.td

                

    def run(self):
        while True:
            # Accept a message from the input channel
            message = self.inputChannel.subscribe()

            # Get the command type
            moveType = message.getBytes('movementType')
            #print(moveType)

            if moveType[0] == b'V'[0]:
                # Copy into the velocity
                for i in range(6):
                    self.jointSpeeds[i] = message.getFloat('jointSetValues',i)

            elif moveType[0] == b'P'[0]:
                # Copy the movement into the target
                for i in range(6):
                    self.targetJointPositions[i] = message.getFloat('jointSetValues',i)
            



if __name__ == "__main__":
    Arm = FakeArm()
    Arm.run()