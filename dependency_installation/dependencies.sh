#!/bin/bash

echo "installing dependencies..."

sudo dnf install -y libevdev libevdev-devel

if ! lsmod | grep -q uinput; then
    echo "loading uinput kernel module..."
    sudo modprobe uinput
else
    echo "uinput module already loaded."
fi

if [ ! -e /dev/uinput ]; then
    echo "/dev/uinput not found, enable uinput in kernel config."
else
    echo "/dev/uinput is available."
fi

echo "dependencies installed!"
