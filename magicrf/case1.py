# -*- coding:utf-8 -*-
""" Case1 """
# !/usr/bin/python
# Python:   3.6.5
# Platform: Windows/Linux/ARMv7
# Author:   Heyn (heyunhuan@gmail.com)
# Program:  Read tag EPC.
# History:  2018-10-30 Ver:1.0 [Heyn] Initialization

import sys
import time
import queue
import datetime
import threading
from magicrf import m100


import serial.tools.list_ports
PLIST = list(serial.tools.list_ports.comports())
if (len(PLIST) <= 0):
    sys.stderr.write('Could not find serial port !!!\n')
    sys.exit(1)

SERIAL_PORT = list(PLIST[0])[0]
print(SERIAL_PORT)


QUEUE_READER = queue.Queue(2048)


READER = m100(SERIAL_PORT)

def receive_callback(data):
    QUEUE_READER.put(data)


READER.rxcallback( receive_callback )
READER.start()


def realtime_threading( queue ):
    data = ''
    while True:
        data = queue.get()
        for item in data.split(';'):
            if not item:
                continue
            try:
                epc, rssi = item.split(',')
            except ValueError:
                print(item)
                continue
            timenow = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            print('{0} -> {1} RSSI: -{2} dBm'.format(timenow, epc, int(rssi, 16)))
    queue.task_done()

REALTIME_THD = threading.Thread( target=realtime_threading, args=( QUEUE_READER, ) )
REALTIME_THD.setDaemon(True)
REALTIME_THD.start()


for _ in ( READER.power(22), READER.mode(), READER.hfss(m100.HFSS_AUTO), READER.param(q=4) ):
    time.sleep(0.1)

while True:
    READER.query(500)
    time.sleep(0.01)
