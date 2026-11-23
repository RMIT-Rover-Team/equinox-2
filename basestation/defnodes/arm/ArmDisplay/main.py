#!/usr/bin/env python3
import pybullet as p
import pybullet_data
import threading

# Import your channel type(s)
from DEFCOM import ChannelLossyCast

#Joint Mappings:
ArmJoints = {
    0: 'rev_j0',
    1: 'rev_j1',
    2: 'rev_j2',
    3: 'rev_j3',
    4: 'rev_j4',
    5: 'rev_j5'
}




def main():
    p.connect(p.GUI)
    p.setAdditionalSearchPath(pybullet_data.getDataPath())
    p.setTimeStep(1/30)

    # Load the URDF
    robot = p.loadURDF("myactuator_arm_assembly/urdf/myactuator_arm_assembly.urdf", useFixedBase=True)
    #robot = p.loadURDF("simplified.urdf", useFixedBase=True)


    # Dump the Joints loaded and build the lookup
    JointLookup = {}
    print("Loaded the following joints from urdf:")
    for i in range(p.getNumJoints(robot)):
        info = p.getJointInfo(robot, i)
        print(i, info[1].decode("utf-8"))
        JointLookup[info[1].decode("utf-8")] = i

    


    # Create the DEFCOM Connection
    channel = ChannelLossyCast.LossyCastSubscriber('COMFILES/ArmState.defcom')



    #Create a crosshair
    target = [0,0,0]
    size = 0.02
    x_id = p.addUserDebugLine([0,0,0], [0,0,0], [1,0,0], 3)
    y_id = p.addUserDebugLine([0,0,0], [0,0,0], [1,0,0], 3)
    z_id = p.addUserDebugLine([0,0,0], [0,0,0], [1,0,0], 3)
    def targetSubLoop():
        target = [0,0,0]
        targetchan = ChannelLossyCast.LossyCastSubscriber('COMFILES/ArmIKControl.defcom')

        while True:
            m = targetchan.subscribe()
            target[0] = m.getFloat('endPosition',0)
            target[1] = m.getFloat('endPosition',1)
            target[2] = m.getFloat('endPosition',2)

            # Update crosshair lines
            p.addUserDebugLine([target[0]-size, target[1], target[2]],
                               [target[0]+size, target[1], target[2]],
                               [1,0,0], 3, replaceItemUniqueId=x_id)

            p.addUserDebugLine([target[0], target[1]-size, target[2]],
                               [target[0], target[1]+size, target[2]],
                               [1,0,0], 3, replaceItemUniqueId=y_id)

            p.addUserDebugLine([target[0], target[1], target[2]-size],
                               [target[0], target[1], target[2]+size],
                               [1,0,0], 3, replaceItemUniqueId=z_id)

    t = threading.Thread(target=targetSubLoop)
    t.start()

      

    #The main loop
    while True:
        # Get the message
        newPos = channel.subscribe()

        # Get the Joint Positions
        for i in range(6):
            print("Joint",i,ArmJoints[i],JointLookup[ArmJoints[i]], newPos.getFloat("jointPositions",i))
            p.resetJointState(robot, JointLookup[ArmJoints[i]], newPos.getFloat("jointPositions",i))

        #Update the simulation
        #p.stepSimulation()
if __name__ == "__main__":
    main()
