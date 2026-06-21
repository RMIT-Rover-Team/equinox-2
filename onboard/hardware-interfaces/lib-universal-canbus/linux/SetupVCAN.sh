#!/bin/bash
sudo modprobe vcan
sudo ip link add vcan0 type vcan
sudo ip link set vcan0 up