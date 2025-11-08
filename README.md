# RAT3 Diff Drive
RAT3 ROS2 Control Integration and Canbus Hardware Interface for Differential Drive Robot with 6 wheels.

Use the following commands to build and launch with the bringup package.

```
colcon build
ros2 launch rat3_bringup launch_robot.launch.py
```

Important commands for setting up Canbus:

```
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
sudo ip link set can0 txqueuelen 1000
```

After launch, use the following command for teleop control:

```
ros2 run teleop_twist_keyboard teleop_twist_keyboard -r cmd_vel:=/diff_cont/cmd_vel_unstamped
```