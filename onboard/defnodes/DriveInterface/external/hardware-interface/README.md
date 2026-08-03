# (Websocket)
To Build:
```
bash build_pycontrol.sh
```
To enable virtual CAN for testing:
```
bash SetupVCAN.sh
```
Alternatively, The torque controller can be configured to use fakemotor odrive emulators in header files in include/

To Run:
```
cd dist/
python3 main.py <caninterface>
```
Such as:
```
python3 main.py can0
```


# Direct Drive
Build files under test/
For each test dir:
```
make -j4
./TestDrive <caninterface>
./TestDrive can0
```


# (ROS) RAT3 Diff Drive
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




