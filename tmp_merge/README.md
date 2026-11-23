## 2026-2027 URC Arm IK model
using ikpy package python

and DEFCOM backend


# Getting Started
To install the arm code: Execute the following commands in your terminal:

```bash
sudo apt install curl -y
curl https://raw.githubusercontent.com/RMIT-Rover-Team/urc-arm-ik/refs/heads/main/Quickstart.sh | bash

```

Connect your XBox controller to your computer,
then:

``bash
cd ARM/urc-arm-ik
defcombuild cleanall
defcombuild all
defcomlaunch LaunchTraining.defcom
```

It will take a moment for the arm to load