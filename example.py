#!/usr/bin/env python

from Can import *

can_start()
raw = RawCan()
raw.raw_can_map_channel('my_channel_1')
raw.raw_can_send('my_channel_1', '01 23 XX 45')


