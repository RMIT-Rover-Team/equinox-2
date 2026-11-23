#!/bin/sh

# Remove existing install if it is there
rm -rvf ARM

mkdir ARM
cd ARM

sudo apt install git gcc g++ python3 python3-pip python3-venv cmake make git libsdl2-dev -y
git clone https://github.com/RMIT-Rover-Team/DEFCOM.git

cd DEFCOM
sudo bash install.sh

cd ..
git clone https://github.com/RMIT-Rover-Team/urc-arm-ik.git
cd urc-arm-ik
/usr/sbin/defcombuild cleanall # I know that sometimes I accidentally commit binaries!
/usr/sbin/defcombuild all
