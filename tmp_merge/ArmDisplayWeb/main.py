import time

import viser
from viser.extras import ViserUrdf
from yourdfpy import URDF
import numpy as np
from DEFCOM import ChannelLossyCast
import threading

joint_map = {
    "rev_j0": 27,
    "rev_j1": 28,
    "rev_j2": 29,
    "rev_j3": 30,
    "rev_j4": 31,
    "rev_j5": 32,
}

class app:
    def __init__(self):
        self.server = viser.ViserServer()

        self.robot_urdf = URDF.load(
            "myactuator_arm_assembly/urdf/myactuator_arm_assembly.urdf"
        )

        print("Actuated joints:")
        for i, name in enumerate(self.robot_urdf.actuated_joint_names):
            print(i, name)

        self.robot = ViserUrdf(
            self.server,
            self.robot_urdf,
        )

        # Add the IK Target
        self.target = self.server.scene.add_frame(
            "/target",
            axes_length=0.1,
            axes_radius=0.003,
            position=(0.0, 0.0, 0.0),
        )


        self.channel = ChannelLossyCast.LossyCastSubscriber(
            "COMFILES/ArmState.defcom"
        )

        self.targetChannel = ChannelLossyCast.LossyCastSubscriber(
            "COMFILES/ArmIKControl.defcom"
        )

        self.targetSub = threading.Thread(target=self._targetListener)
        self.targetSub.start()

        self.targetPos = (0,0,0)

    def _targetListener(self):
        while True:
            msg = self.targetChannel.subscribe()
            self.targetPos = (msg.getFloat('endPosition',0),msg.getFloat('endPosition',1),msg.getFloat('endPosition',2))


    def run(self):
        while True:
            msg = self.channel.subscribe()

            # Update the arm pos
            q = np.zeros(len(self.robot_urdf.actuated_joint_names))

            q[27] = msg.getFloat("jointPositions", 0)
            q[28] = msg.getFloat("jointPositions", 1)
            q[29] = msg.getFloat("jointPositions", 2)
            q[30] = msg.getFloat("jointPositions", 3)
            q[31] = msg.getFloat("jointPositions", 4)
            q[32] = msg.getFloat("jointPositions", 5)

            # Update the target crosshair
            self.target.position = self.targetPos

            self.robot.update_cfg(q)

if __name__ == "__main__":
    app().run()