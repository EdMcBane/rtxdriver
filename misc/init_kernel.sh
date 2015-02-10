#!/bin/bash

sudo mount -t usbfs -o uid=0,gid=plugdev stub_usbfs /proc/bus/usb/
sudo mount -t debugfs stub_debugfs /sys/kernel/debug/
sudo modprobe usbmon

