import pybullet as p
import pybullet_data
import math
import time


p.connect(p.GUI)
p.setAdditionalSearchPath(pybullet_data.getDataPath())

robot = p.loadURDF("myactuator_arm_assembly/urdf/myactuator_arm_assembly.urdf", useFixedBase=True)

# Example pose (in radians)
joint_angles = [
    10,     # J0 yaw
    45,     # J1 pitch
    45,    # J2 pitch
    0.8,     # J3 pitch
    0.0,     # J4 yaw
    1.2      # J5 roll
]

for i in range(p.getNumJoints(robot)):
    info = p.getJointInfo(robot, i)
    print(i, info[1].decode("utf-8"))


for i, angle in enumerate(joint_angles):
    p.resetJointState(robot, i, math.radians(angle))

while True:
    p.stepSimulation()
    ID = int(input("Joint: "))
    Val = int(input("Value: "))
    p.resetJointState(robot,ID,math.radians(Val))
