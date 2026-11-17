#!/usr/bin/env python3
import ikpy
from ikpy.chain import Chain
from ikpy.link import URDFLink
#from urdfpy import URDF
from DEFCOM import ChannelLossyCast
import numpy as np
from ikpy.utils import geometry
import time
import pybullet as p
import pybullet_data
import threading

ArmJoints = [
    'rev_j0',
    'rev_j1',
    'rev_j2',
    'rev_j3',
    'rev_j4',
    'rev_j5'
]

class motionplanner:
    def __init__(self):
        print("Loading URDF")
        self.chain = self.convertURDFtoChain()
        self.joint_index = {link.name: i for i, link in enumerate(self.chain.links)}
        print("URDF Loaded and converted to chain")

        self.armJointPos = [0]*len(ArmJoints)


    def setJointOrigins(self, joints):
        self.armJointPos = joints


    # Cartesian X, Y, Z for the arm to home to
    def translatePosToAngles(self,x,y,z) -> list[float]:
        angles = self.chain.inverse_kinematics([x,y,z],initial_position=self.armJointPos)
        oangles = []
        for name in ArmJoints:
            oangles.append(
                angles[self.joint_index[name]]
                )

        print(oangles)
        return oangles

    # Cartesian X Y Z with orientation too as roll pitch yaw
    def translatePosOrientToAngles(self, x, y, z, rl, pt, yw):
        #Need to convert to a rotation matrix 4x4
        rot = geometry.rpy_matrix(rl, pt, yw)

        target_frame = np.eye(4)
        target_frame[:3, :3] = rot
        target_frame[:3, 3] = [x, y, z]

        angles = self.chain.inverse_kinematics_frame(
            target=target_frame,
            orientation_mode="all",
            initial_position=self.armJointPos
        )

        oangles = []
        for name in ArmJoints:
            oangles.append(
                angles[self.joint_index[name]]
                )

        print(oangles)
        return oangles


    #IKPY needs a special format so we need to convert from urdf
    def convertURDFtoChain(self):
        p.connect(p.DIRECT)
        robot = p.loadURDF("simplified.urdf", useFixedBase=True)

        links = []

        for j in range(p.getNumJoints(robot)):
            info = p.getJointInfo(robot, j)

            joint_name = info[1].decode()
            joint_type = info[2]

            # Skip fixed joints
            if joint_type == p.JOINT_FIXED:
                continue

            # Local joint origin (correct!)
            joint_pos = info[14]   # xyz
            joint_rpy = info[15]   # rpy
            joint_axis = info[13]  # axis

            links.append(
                URDFLink(
                    name=joint_name,
                    origin_translation=joint_pos,
                    origin_orientation=[0,0,0],
                    rotation=joint_axis
                )
            )

        

        return Chain(name="robot_chain", links=links)



class IKServer:
    def __init__(self, planner: motionplanner):
        self.inChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/ArmIKControl.defcom")
        self.outChannel = ChannelLossyCast.LossyCastPublisher("COMFILES/ArmControl.defcom")
        self.feedbackChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/ArmState.defcom")
        self.planner = planner

        self.fThread = threading.Thread(target=self._armStateListener, args=())
        self.fThread.start()


    def _armStateListener(self):
        while True:
            msg = self.feedbackChannel.subscribe()
            pos = [msg.getFloat("jointPositions",i) for i in range(len(ArmJoints))]
            self.planner.setJointOrigins(pos)

    def run(self):
        last = time.time()
        while True:
            target = self.inChannel.subscribe()

            # We throttle to 100ms
            now = time.time()
            if (now - last) > 0.1:
                last = now

                #Compute the angles
                angles = self.planner.translatePosOrientToAngles(
                    target.getFloat('endPosition',0), target.getFloat('endPosition',1), target.getFloat('endPosition',2), 
                    target.getFloat('endOrientation',0), target.getFloat('endOrientation',1), target.getFloat('endOrientation',2)
                    )
                #angles = self.planner.translatePosToAngles(target.getFloat('endPosition',0), target.getFloat('endPosition',1), target.getFloat('endPosition',2))

                outgoing = self.outChannel.getNewMessageObject()

                for i, a in enumerate(angles):
                    outgoing.setFloat("jointSetValues",a,i)
                print(angles)
                outgoing.setBytes("movementType",b'P')

                #Send the message
                self.outChannel.publish(outgoing)


if __name__ == "__main__":
    plan = motionplanner()

    print("Self Test")
    plan.translatePosToAngles(1,2,3)
    plan.translatePosOrientToAngles(4,5,6,0.4, 0.3, 0.1)

    print("Starting Comms")
    srv = IKServer(plan)
    srv.run()

