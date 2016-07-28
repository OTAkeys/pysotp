#!/bin/bash

set -e

function get_iface {
    ifconfig $1 | sed 's/[ \t].*//;/^$/d' | grep can
}

# check if interface exists
if ! IFACE=$(get_iface -a); then
    echo "No can interface found. Is the USB interface plugged in?"
    exit 1
else
    echo $IFACE
fi
