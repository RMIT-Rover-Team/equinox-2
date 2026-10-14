dtc -@ -I dts -O dtb -o mcpeq2.dtbo mcpeq2.dts
sudo cp mcpeq2.dtbo /boot/firmware/overlays/

#Add to /boot/config
# dtoverlay=mcpeq2,can0=on
# dtoverlay=mcpeq2,can1=on

