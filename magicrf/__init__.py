# -*- coding:utf-8 -*-
""" Library for UHF RFID Soc Reader Chip M100 and QM100 """
# !/usr/bin/python
# Python:   3.6.5
# Platform: Windows/Linux/ARMv7
# Author:   Heyn (heyunhuan@gmail.com)
# Program:  Library for UHF RFID Soc Reader Chip M100 and QM100.
# History:  2018-10-29 Wheel Ver:1.0.0 [Heyn] Initialize
#           2018-11-26 Wheel Ver:1.0.1 [Heyn] Optimized code.

__author__    = 'Heyn'
__copyright__ = 'Hexin'
__version__   = '1.0.1'

import serial
import serial.threaded

from .CONSTANT import *
from magicrf import _m100
from magicrf._m100 import Protocol

class HexinProtocolFactory( serial.threaded.FramedPacket ):

    def __init__(self, unpack=None):
        super( HexinProtocolFactory, self ).__init__()
        self.__unpack = unpack

    def __call__(self):
        return self

    @property
    def trigger(self):
        return self.__trigger
    
    @trigger.setter
    def trigger(self, value):
        self.__trigger = value

    def data_received(self, data):
        self.__unpack( data, self.__trigger )


def uart_register(func):
    def wrapper(self, *args, **kwargs):
        payload = func(self, *args, **kwargs)
        return self.ser.write(payload)
    return wrapper


class m100( Protocol ):
    HFSS_AUTO = _m100.HFSS_AUTO
    HFSS_STOP = _m100.HFSS_STOP

    MODE_HIGH_SENSITIVITY = _m100.MODE_HIGH_SENSITIVITY
    MODE_DENSE_READER     = _m100.MODE_DENSE_READER

    def __init__(self, port='COM1', baudrate=115200, bytesize=8, parity=serial.PARITY_NONE, stop=serial.STOPBITS_ONE):
        super( m100, self ).__init__()
        self.ser = serial.serial_for_url(port, do_not_open=True)
        self.ser.baudrate = baudrate
        self.ser.bytesize = bytesize
        self.ser.parity   = parity
        self.ser.stopbits = stop
        self.open()

    def __repr__(self):
        return 'Port:{0},{1},{2},{3},{4}'.format( self.ser.name,
                                                  self.ser.baudrate,
                                                  self.ser.bytesize,
                                                  self.ser.parity,
                                                  self.ser.stopbits )

    def open(self):
        try:
            self.ser.open()
        except serial.SerialException as err:
            raise FileNotFoundError('Could not open serial port {}: {}'.format(self.ser.name, err))

    def rxcallback(self, func):
        self.unpack_cb( func )

    @uart_register
    def query(self, loop=1):
        return super( m100, self ).query(loop)

    @uart_register
    def power(self, dbm=None):
        # self.__protocol.trigger = _m100.TRIGGER_GET_PA_POWER if dbm == None else _m100.TRIGGER_SET_PA_POWER
        return super( m100, self ).power() if dbm == None else super( m100, self ).power(dbm)

    @uart_register
    def mode(self, param=_m100.MODE_DENSE_READER):
        return super( m100, self ).mode(param)

    @uart_register
    def hfss(self, mode=_m100.HFSS_AUTO):
        return super( m100, self ).hfss(mode)

    @uart_register
    def insert(self, start=0, stop=5):
        return super( m100, self ).insert(start=start, stop=stop)

    @uart_register
    def setchannel(self, index=1):
        return super( m100, self ).setchannel(index)

    @uart_register
    def param(self, select=PARAM_SELECT['ALL'], session=PARAM_SESSION['S0'], target=PARAM_TARGET['A'], q=4):
        return super( m100, self ).param(select=select, session=session, target=target, q=q)

    @uart_register
    def select(self, mask, target=PARAM_TARGET['A'], action=0, bank=MEMORY_BANK['EPC'], ptr=2, truncate=False):
        return super( m100, self ).select(mask, target=target, action=action, bank=bank, ptr=ptr, truncate=truncate)
    
    @uart_register
    def read(self, memory=MEMORY_BANK['USER'], length=2, address=0 ):
        """ @param length   : Read size
            @param memory   : Tag memory bank
            @param address  : Offset address
            @param password : Password
        """

        return super( m100, self ).read(memory=memory, address=address, length=length)

    @uart_register
    def write(self, data, memory=MEMORY_BANK['USER'], address=0):
        """ @param data     : Write data
            @param memory   : Tag memory bank
            @param address  : Offset address
            @param password : Password
        """
        return super( m100, self ).write(data=data, memory=memory, address=address)

    @uart_register
    def write_epc(self, data):
        """ @param data     : EPC data
            @param password : (default:00000000) It's must be 8Bytes
        """
        return self.epc(data=data)

    @uart_register
    def setDemodulator(self, mixer=3, ifgain=6, thrd=0x01B0):
        return self.demodulator( mixer=mixer, ifgain=ifgain, thrd=thrd )

    def start(self, callback=HexinProtocolFactory, trigger=TRIGGER['QUERY']):
        self.__protocol = HexinProtocolFactory( self.unpack )
        self.__protocol.trigger = trigger
        serial_worker = serial.threaded.ReaderThread( self.ser, self.__protocol )
        serial_worker.start()
