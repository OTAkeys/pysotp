import os
from ctypes import *
from channels import RAW_CHANNELS, ISO_TP_CHANNELS
from datalist import DataList
from subprocess import Popen, PIPE, STDOUT
from time import sleep

OK = 0
ERR = -1
ERR_TIMEOUT = -2

required_dll = os.path.join(os.path.dirname(__file__), 'libcanwrap.so')
dll = cdll.LoadLibrary(required_dll)

logger = None
DEVNULL = None


def convert_name(name):
    return name.lower().split('.')[-1].replace(' ', '_')


def can_start(suite_name=''):
    # get can interface
    proc = Popen(['./check_iface.sh'], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
    iface = proc.communicate()[0].strip()
    if proc.returncode != 0:
        raise Exception(iface)
    # initialize
    ret = dll.raw_can_init(iface, len(iface))
    if ret < 0:
        raise Exception("Error on initialization")
    ret = dll.iso_tp_init(iface, len(iface))
    if ret < 0:
        raise Exception("Error on initialization")
    # start logging
    global DEVNULL
    DEVNULL = open(os.devnull, 'wb')
    global logger
    logger = Popen(['tshark', '-i', iface, '-w', 'output/can_{}.pcap'.format(convert_name(suite_name))], stdout=DEVNULL,
                   stderr=STDOUT)
    sleep(1)  # wait for logger to start


def can_stop():
    dll.iso_tp_stop()
    dll.raw_can_stop()
    logger.terminate()
    DEVNULL.close()


class IsoTp:
    ROBOT_LIBRARY_SCOPE = 'GLOBAL'

    def __init__(self):
        self.ch_ids = {}

    def iso_tp_map_channel(self, channel):
        try:
            tx = ISO_TP_CHANNELS[channel][0]
            rx = ISO_TP_CHANNELS[channel][1]
        except KeyError:
            raise Exception("Invalid iso-tp channel")
        ret = dll.iso_tp_map_channel(tx, rx)
        if ret < 0:
            raise Exception("Error on mapping channel: {}".format(ret))
        self.ch_ids[channel] = ret

    def iso_tp_flush_rx(self, channel):
        try:
            ch_id = self.ch_ids[channel]
        except KeyError:
            raise Exception("Unmapped channel")
        ret = dll.iso_tp_flush_rx(ch_id)
        if ret < 0:
            raise Exception("Error flushing: {}".format(ret))

    def iso_tp_send(self, channel, data):
        data = DataList.from_string(data)
        try:
            ch_id = self.ch_ids[channel]
        except KeyError:
            raise Exception("Unmapped channel")
        ret = dll.iso_tp_send(ch_id, data.to_bytes(), len(data))
        if ret < 0:
            raise Exception("Error sending: {}".format(ret))

    def iso_tp_receive_and_check(self, channel, data, timeout):
        ch_id = self.ch_ids[channel]
        rec_data = create_string_buffer(4095)
        rec_len = c_int()
        ret = dll.iso_tp_receive(ch_id, rec_data, byref(rec_len), int(timeout))
        if ret == ERR:
            raise Exception("Error receiving")
        elif ret == ERR_TIMEOUT:
            raise Exception("Timeout expired without receiving data")
        rec_data = DataList.from_bytes(rec_data.raw[:rec_len.value])
        exp_data = DataList.from_string(data)
        if rec_data != exp_data:
            raise Exception("\nExpected data: {}\nReceived data: {}".format(exp_data, rec_data))
        else:
            return str(rec_data)


class RawCan:
    ROBOT_LIBRARY_SCOPE = 'GLOBAL'

    def __init__(self):
        self.ch_ids = {}

    def raw_can_flush_rx(self, channel):
        try:
            ch_id = self.ch_ids[channel]
        except KeyError:
            raise Exception("Unmapped channel")
        ret = dll.raw_can_flush_rx(ch_id)
        if ret < 0:
            raise Exception("Error flushing: {}".format(ret))

    def raw_can_map_channel(self, channel):
        try:
            can_id = RAW_CHANNELS[channel]
        except KeyError:
            raise Exception("Invalid raw can channel")
        ret = dll.raw_can_map_channel(can_id)
        if ret < 0:
            raise Exception("Error on mapping channel: {}".format(ret))
        self.ch_ids[channel] = ret

    def raw_can_send(self, channel, data):
        data = DataList.from_string(data)
        try:
            ch_id = self.ch_ids[channel]
        except KeyError:
            raise Exception("Unmapped channel")
        ret = dll.raw_can_send(ch_id, data.to_bytes(), len(data))
        if ret < 0:
            raise Exception("Error sending: {}".format(ret))

    def raw_can_receive_and_check(self, channel, data, timeout):
        ch_id = self.ch_ids[channel]
        rec_data = create_string_buffer(8)
        rec_len = c_int()
        ret = dll.raw_can_receive(ch_id, rec_data, byref(rec_len), int(timeout))
        if ret == ERR:
            raise Exception("Error receiving")
        elif ret == ERR_TIMEOUT:
            raise Exception("Timeout expired without receiving data")
        rec_data = DataList.from_bytes(rec_data.raw[:rec_len.value])
        exp_data = DataList.from_string(data)
        if rec_data != exp_data:
            raise Exception("\nExpected data: {}\nReceived data: {}".format(exp_data, rec_data))
        else:
            return str(rec_data)
