
from random import randint
import struct
import copy

def int_to_byte(number):
    return struct.pack("<B", number)

def byte_to_int(byte):
    return struct.unpack("<B", byte)[0]


class DataList(list):
    def __init__(self, *args):
        list.__init__(self, *args)

    @classmethod
    def from_string(cls, string):
        l = []
        if string:
            for el in string.split(' '):
                try:
                    l.append(int(el, 16))
                except ValueError:
                    l.append(el)
        return cls(l)

    @classmethod
    def from_bytes(cls, string):
        l = []
        if string:
            for el in string:
                l.append(byte_to_int(el))
        return cls(l)

    def to_bytes(self):
        return str(bytearray(self.for_sending()))

    def for_sending(self):
        l = copy.copy(self)
        for index, el in enumerate(l):
            if type(el) is not int:
                l[index] = randint(0, 255)
        return l

    def __eq__(self, other):
        if len(self) != len(other):
            return False
        for index, el in enumerate(self):
            if type(el) is not int or type(other[index]) is not int or el == other[index]:
                continue
            else:
                return False
        else:
            return True

    def __ne__(self, other):
        return not self.__eq__(other)

    def __str__(self):
        l = []
        for el in self:
            try:
                l.append('{:02x}'.format(el))
            except ValueError:
                l.append(str(el))
        return ' '.join(l)


