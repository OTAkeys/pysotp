

# Here you should define the raw can and isotp channels that you want to use.

# This maps a channel name to a single CAN ID
RAW_CHANNELS = {
    'my_channel_1': 0x001,
    'my_channel_2': 0x005,
}

# This maps a channel name to a set of CAN ID's used for tx and rx
ISO_TP_CHANNELS = {
    'iso_tp_channel_1': (0x042, 0x043),
    'iso_tp_channel_2': (0x001, 0x002),
    'iso_tp_channel_3': (0x003, 0x004),
}

