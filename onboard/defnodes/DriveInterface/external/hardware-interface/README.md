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

# Mini Rover GPIO drive

The DriveInterface node can drive the mini rover's four H-bridge channels with
`libgpiod` v2 instead of WiringPi. Its existing DEFCOM controller input remains
unchanged; only the motor-output backend changes.

Build this backend on the Raspberry Pi with:

```
cmake -S onboard/defnodes/DriveInterface -B build/minirover -DDRIVE_USE_MINIROVER_GPIO=ON
cmake --build build/minirover
```

Install the `libgpiod` v2 development package first (for example,
`libgpiod-dev` on Debian-based Raspberry Pi images). The process needs access
to `/dev/gpiochip0`, normally through an appropriate GPIO group or a udev rule.

The adapter preserves the old `wiringPiSetup()` numbering by using these BCM
GPIO line offsets: LF `13/12`, LR `19/16`, RF `18/17`, RR `15/14`; enable
lines `27, 21, 20, 4, 2`. The backend uses a 100 Hz software-PWM loop for
the direction pairs and sets all enable lines high only while the drive is
enabled. It does not supply encoder feedback, so avoid locked-velocity mode.


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


