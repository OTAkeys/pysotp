#!/bin/bash

set -e

BITRATE=500000

if [ $# -ne 0 ]; then
    IFACE=$1
else
    exit 1
fi

# check if can kernel module is loaded
if ! lsmod | grep '^can ' > /dev/null 2>&1; then
    modprobe can
fi

# check if ISO-TP kernel module is loaded
if ! lsmod | grep '^can_isotp ' > /dev/null 2>&1; then
    modprobe can-isotp
fi

# configure restart
sudo ip link set $IFACE type can restart-ms 100

# configure baudrate
sudo ip link set $IFACE up type can bitrate $BITRATE

