# Pysotp
## A python library for raw can and iso-tp over Socketcan

Originally this library was used as a can library for robot framework, but it can also be used as a general iso-tp and raw can library for python.

Usage:
```
cd can_wrap_src
make
cd ..
./example.py
```



NOTE:

This uses socketCAN (available as from kernel V2.6.25) with the experimental ISO-TP driver and can-utils user space utilities.

For installing the ISO-TP driver:
$ git clone https://github.com/hartkopp/can-isotp-modules
$ cd can-isotp-modules/net/can
$ ./make_isotp.sh

1. MANUAL LOADING OF DRIVERS

Make sure socketCAN is loaded, by plugging in a CAN interface or running:
$ modprobe can

Load the ISO-TP kernel module:
$ insmod can-isotp.ko

For installing can-utils:
$ git clone https://github.com/linux-can/can-utils
$ cd can-utils
$ make
$ make install

Set the can interface up:
$ ip link set can0 up

2. LOADING DRIVERS & SETUP IFACE AUTOMATICALLY

Copy compiled can-isotp.ko module to modules directory:
$ cp can-isotp.ko /lib/modules/$(uname -r)/kernel/net/can/

Find dependencies for kernel modules:
$ depmod -a

Now the can-isotp module can be loaded with modprobe instead of insmod.

Copy the udev rule and corresponding script to the right directories:
$ cp 90-can-interface.rules /etc/udev/rules.d/
$ cp setup_can_iface.sh /usr/local/bin/

Now setup_can_iface.sh script is run by udev when a can interface is plugged in. This script makes sure the correct drivers are loaded and the interface is set to up.

CAVEAT: When a new kernel version is installed, you might have to copy the the can-isotp.ko kernel module again to the modules directory.
