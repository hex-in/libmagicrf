# -*- coding:utf-8 -*-
""" M100 module constant """
# !/usr/bin/python
# Python:   3.6.5
# Platform: Windows/Linux/ARMv7
# Author:   Heyn (heyunhuan@gmail.com)
# Program:  M100 module constant.
# History:  2018-11-09 Wheel Ver:1.0.0 [Heyn] Initialize

from magicrf import _m100

PARAM_TARGET  = { 'A' : _m100.PARAM_TARGET_A,
                  'B' : _m100.PARAM_TARGET_B }

PARAM_SELECT  = { 'ALL' : _m100.PARAM_SELECT_ALL,
                  'NSL' : _m100.PARAM_SELECT_NSL,
                  'SL'  : _m100.PARAM_SELECT_SL }

PARAM_SESSION = { 'S0': _m100.PARAM_SESSION_S0,
                  'S1': _m100.PARAM_SESSION_S1,
                  'S2': _m100.PARAM_SESSION_S2,
                  'S3': _m100.PARAM_SESSION_S3 }

PARAMETERS    = { 'target'  : PARAM_TARGET,
                  'select'  : PARAM_SELECT,
                  'session' : PARAM_SESSION }

MEMORY_BANK   = { 'RFU' : _m100.BANK_RFU,
                  'EPC' : _m100.BANK_EPC,
                  'TID' : _m100.BANK_TID,
                  'USER': _m100.BANK_USER }

PARAM_REGION  = { 'china900' : _m100.REGION_CHINA_900,
                  'china800' : _m100.REGION_CHINA_800,
                  'europe'   : _m100.REGION_EUROPE,
                  'america'  : _m100.REGION_AMERICA,
                  'korea'    : _m100.REGION_KOREA }

PARAM_MIXER   = { '0db' : _m100.MIXER_GAIN_0DB,
                  '3db' : _m100.MIXER_GAIN_3DB,
                  '6db' : _m100.MIXER_GAIN_6DB,
                  '9db' : _m100.MIXER_GAIN_9DB,
                  '12db': _m100.MIXER_GAIN_12DB,
                  '15db': _m100.MIXER_GAIN_15DB,
                  '16db': _m100.MIXER_GAIN_16DB }

PARAM_IF_GAIN = { '12db': _m100.IF_GAIN_12DB,
                  '18db': _m100.IF_GAIN_18DB,
                  '21db': _m100.IF_GAIN_21DB,
                  '24db': _m100.IF_GAIN_24DB,
                  '27db': _m100.IF_GAIN_27DB,
                  '30db': _m100.IF_GAIN_30DB,
                  '36db': _m100.IF_GAIN_36DB,
                  '40db': _m100.IF_GAIN_40DB }

RFCH_FREQ_CHINA900 = [ round(i*0.001, 3) for i in range(920125, 925125, 250) ]
RFCH_FREQ_CHINA800 = [ round(i*0.001, 3) for i in range(840125, 845125, 250) ]
RFCH_FREQ_AMERICA  = [ round(i*0.01,  2) for i in range(90225, 92825, 50) ]
RFCH_FREQ_EUROPE   = [ round(i*0.1,   1) for i in range(8651,  8681,  2) ]
RFCH_FREQ_KOREA    = [ round(i*0.1,   1) for i in range(9171,  9235,  2) ]

TRIGGER = { 'QUERY'     : _m100.TRIGGER_QUERY,
            'STOP'      : _m100.TRIGGER_STOP,
            'GET'       : { 'PA_POWER'  : _m100.TRIGGER_GET_PA_POWER,    },
            'SET'       : { 'PA_POWER'  : _m100.TRIGGER_SET_PA_POWER,    },
}
