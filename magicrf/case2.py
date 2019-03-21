# -*- coding:utf-8 -*-
""" Case2 """
# !/usr/bin/python
# Python:   3.6.5
# Platform: Windows/Linux/ARMv7
# Author:   Heyn (heyunhuan@gmail.com)
# Program:  Read tag EPC.
# History:  2018-10-30 Ver:1.0 [Heyn] Initialization

import time
import queue
import threading
from magicrf import m100

SERIAL_PORT = 'COM14'

QUEUE_READER = queue.Queue(2048)

READER = m100(SERIAL_PORT)

def receive_callback(data):
    QUEUE_READER.put(data)

READER.rxcallback( receive_callback )
READER.start()


def realtime_threading( queue ):
    tags = dict()
    while True:
        try:
            data = queue.get(timeout=1)
        except BaseException:
            data = ';'
            pass

        for item in data.split(';'):
            if not item:
                continue
            try:
                epc, rssi = item.split(',')
            except ValueError:
                print(item)
                continue

            tags[epc] = (int(round( time.time() * 1000)), int( rssi, 16 ))

        for key in list(tags.keys()):
            diff = int(round( time.time() * 1000)) -  tags[key][0]
            if diff > 1000:
                print('Timeout -> {0} -- {1} ms'.format(key, diff))
                del tags[key]


REALTIME_THD = threading.Thread( target=realtime_threading, args=( QUEUE_READER, ) )
REALTIME_THD.setDaemon(True)
REALTIME_THD.start()

for _ in ( READER.power(22), READER.mode(), READER.hfss(m100.HFSS_AUTO), READER.param(q=4) ):
    time.sleep(0.1)

while True:
    READER.query(500)
    time.sleep(0.01)
