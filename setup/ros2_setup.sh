#!/bin/bash
#!/usr/bin/env bash
set -e

echo "=== Updating system ==="
sudo apt update && sudo apt upgrade -y
sudo apt install htop btop tmux build-essential gcc g++ make cmake -y

echo "=== Installing required packages ==="
sudo apt install -y locales curl gnupg lsb-release software-properties-common

echo "=== Setting locale ==="
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

echo "=== Adding ROS 2 apt repository ==="
sudo apt install -y curl gnupg2
sudo mkdir -p /etc/apt/keyrings

curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    | sudo gpg --dearmor -o /etc/apt/keyrings/ros-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/ros-archive-keyring.gpg] \
http://packages.ros.org/ros2/ubuntu $(lsb_release -cs) main" \
    | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

echo "=== Updating package index ==="
sudo apt update

echo "=== Installing ROS 2 Jazzy Desktop ==="
sudo apt install -y ros-jazzy-desktop

sudo apt install -y ros-jazzy-ros-base

echo "=== Installing development tools ==="
sudo apt install -y \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-argcomplete \
    build-essential

echo "=== Initializing rosdep ==="
sudo rosdep init || true
rosdep update

echo "=== Adding ROS 2 to .bashrc ==="
if ! grep -q "source /opt/ros/jazzy/setup.bash" ~/.bashrc; then
    echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
fi

echo "=== Done! ==="
echo "Open a new terminal or run:"
echo "source /opt/ros/jazzy/setup.bash"