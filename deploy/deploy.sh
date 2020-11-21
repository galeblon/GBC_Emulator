#!/bin/bash

ES_VERSION="v2.9.4"

apt-get update && apt-get -y upgrade

apt-get install -y libsdl2-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev rapidjson-dev \
libasound2-dev libgles2-mesa-dev build-essential cmake fonts-droid-fallback libvlc-dev \
libvlccore-dev vlc-bin openbox xinit xterm git at x11-xserver-utils

dpkg -i usbmount_0.0.24_all.deb; sudo apt-get install -y -f

shopt -s dotglob
cp -r ./home/* /home/pi
chown -R pi /home/pi

cp 10-usb-insert.rules /etc/udev/rules.d/

cd ../src/
make clean
make gbc

git clone --recursive https://github.com/RetroPie/EmulationStation.git /home/pi/EmulationStation

cd /home/pi/EmulationStation
git checkout $ES_VERSION
cmake -DUSE_MESA_GLES=On .
make -j

