/*******************************************************************************
 * Copyright (c) 2018-2019, Hexin Technology Co. Ltd All rights reserved.
 * Author  : Heyn (heyunhuan@gmail.com)
 * Version : V1.1
 * Web	   : http://www.hex-in.com
 * 
 * LICENSING TERMS:
 * ---------------
 *          New Create at 	2018/08/15 V1.0 [Heyn] Initialization.
 *                          2018/10/24 V1.1 [Heyn] New add any function.
 *
*******************************************************************************/

#include <Python.h>
#include <structmember.h>

#include "lib/_protocol.h"  // < Protocol version v1.4.2 >

static hexin_ring_buffer_t ringbuffer;
static unsigned char *rb_buffer = NULL;       // ringbuffer buufer.
static unsigned char *cb_buffer = NULL;       // Callback buffer.

typedef struct {
    unsigned char   bank;       // Memory bank
    unsigned char   mask;       // password is hex or string format
    const char      *data;      // Write data
    unsigned short  data_len;   // Read or write data length
    const char      *pwd;       // Password 
    unsigned short  pwd_len;    // Password length
    unsigned short  addr;       // Read or write address.
}_m100_rw_packet_t;


static PyObject * _m100_idle( PyObject *self, PyObject *args ) {
    unsigned char mode   = 0x00;
    unsigned char minute = 0x01;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "b|b", &mode, &minute ) ) {
        return NULL;
    }

    length = idle( mode, minute, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_query( PyObject *self, PyObject *args ) {
    unsigned int  loop   = 0x00000001L;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|I", &loop ) ) {
        return NULL;
    }

    length = query( (unsigned short)loop, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_region( PyObject *self, PyObject *args ) {
    unsigned char region = REGION_CHINA_900;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "b", &region ) ) {
        return NULL;
    }

    length = setRegion( (module_region_t)region, param );

    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_pa_power( PyObject *self, PyObject *args ) {
    float         power  = 0.0;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|f", &power) ) {
        return NULL;
    }

    if ( PyTuple_Size( args ) == 0 ) {
        length = getPaPower( param );
    } else {
        length = setPaPower( power, param );
    }
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_reader_mode( PyObject *self, PyObject *args ) {
    unsigned char mode   = 0x00;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &mode ) )
        return NULL;

    length = setMode( mode, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_stop( PyObject *self, PyObject *args ) {
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;

    length = stop( param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_rssi( PyObject *self, PyObject *args ) {
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;

    length = testRSSI( param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_hfss( PyObject *self, PyObject *args ) {
    unsigned char mode   = 0x00;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &mode ) )
        return NULL;

    length = setHFSS( mode, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_lock( PyObject *self, PyObject *args, PyObject* kws ) {
    _m100_rw_packet_t packet = { .pwd  = "00000000", .pwd_len  = 0x0008,
                                 .data = NULL,       .data_len = 0 };

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[] = { "password", "lockdata", NULL };
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "|s#s#", kwlist, &packet.pwd, &packet.pwd_len, &packet.data, &packet.data_len ) )
        return NULL;

    if ( (packet.pwd_len != 0x08) || (packet.data_len != 0x03) ) {
        return NULL;
    }

    length = lock( packet.pwd, packet.data, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_kill( PyObject *self, PyObject *args, PyObject* kws ) {
    _m100_rw_packet_t packet = { .pwd  = "00000000", .pwd_len  = 0x0008 };

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[] = { "password", NULL };
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "|s#", kwlist, &packet.pwd, &packet.pwd_len ) )
        return NULL;

    if ( packet.pwd_len != 0x08 ) {
        return NULL;
    }

    length = kill( packet.pwd, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_read( PyObject *self, PyObject *args, PyObject* kws ) {
    _m100_rw_packet_t packet = { .bank = BANK_USER,
                                 .mask = 1,
                                 .data = NULL,       .data_len = 0x0002,
                                 .pwd  = "00000000", .pwd_len  = 0x0008,
                                 .addr = 0x0000 };

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[] = { "memory", "password", "address", "length", NULL };
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "b|s#HH", kwlist, &packet.bank, &packet.pwd, &packet.pwd_len, &packet.addr, &packet.data_len ) )
        return NULL;

    if ( packet.pwd_len != 0x08 ) {
        return NULL;
    }

    length = readData( packet.pwd, packet.mask, packet.bank, packet.addr, packet.data_len, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_write( PyObject *self, PyObject *args, PyObject* kws ) {
    _m100_rw_packet_t packet = { .bank = BANK_USER,
                                 .mask = 1,
                                 .data = NULL,       .data_len = 0x0000,
                                 .pwd  = "00000000", .pwd_len  = 0x0008,
                                 .addr = 0x0000 };    

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[] = {"data", "memory", "password", "address", NULL};
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "s#|bs#H", kwlist, &packet.data, &packet.data_len, &packet.bank, &packet.pwd, &packet.pwd_len, &packet.addr ) )
        return NULL;

    if ( packet.pwd_len != 8 ) {
        return NULL;
    }

    length = writeData( packet.pwd, packet.mask, packet.bank, packet.addr, packet.data_len, packet.data, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_write_epc( PyObject *self, PyObject *args, PyObject* kws ) {
    _m100_rw_packet_t packet = { .bank = BANK_EPC,
                                 .mask = 1,
                                 .data = NULL,       .data_len = 0x0000,
                                 .pwd  = "00000000", .pwd_len  = 0x0008,
                                 .addr = 0x0000 };    

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[] = {"data", "password", NULL};
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "s#|s#", kwlist, &packet.data, &packet.data_len, &packet.pwd, &packet.pwd_len ) )
        return NULL;

    if ( packet.pwd_len != 8 ) {
        return NULL;
    }

    length = writeEPC( packet.pwd, packet.mask, packet.data, packet.data_len, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_rfcarrier( PyObject *self, PyObject *args ) {
    unsigned char status  = 0xFF;
    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &status ) )
        return NULL;

    length = setRFCarrier( status, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_setchannel( PyObject *self, PyObject *args ) {
    unsigned char index   = 0x01;
    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &index ) )
        return NULL;

    length = setRFChannel( index, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_select_mode( PyObject *self, PyObject *args ) {
    unsigned char mode   = 0x01;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &mode ) )
        return NULL;

    length = setSelectMode( mode, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_demodulator( PyObject *self, PyObject *args, PyObject* kws ) {
    unsigned char mixer  = MIXER_GAIN_9DB;
    unsigned char ifgain = IF_GAIN_36DB;
    unsigned short thrd  = 0x01B0;

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };
    static char* kwlist[] = {"mixer", "ifgain", "thrd", NULL};

    if ( !PyArg_ParseTupleAndKeywords( args, kws, "|bbH", kwlist, &mixer, &ifgain, &thrd ) )
        return NULL;

    length = setRevDemodulatorParam( (module_mixer_gain_t)mixer, (module_if_gain_t)ifgain, thrd, param );
    return Py_BuildValue( "y#", param, length );
}


static PyObject * _m100_param( PyObject *self, PyObject *args, PyObject* kws ) {
    unsigned char select  = 0x00;
    unsigned char session = 0x00;
    unsigned char target  = 0x00;
    unsigned char q       = 0x04;
    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[]={"select", "session", "target", "q", NULL};
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "|bbbb", kwlist, &select, &session, &target, &q ) )
        return NULL;

    length = setQueryParam( select, session, target, q, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_select( PyObject *self, PyObject *args, PyObject* kws ) {
    const char *  mask     = NULL;
    unsigned char target   = 0x00;
    unsigned char action   = 0x00;
    unsigned char bank     = BANK_EPC;
    unsigned char truncate = 0x00;
    unsigned int  ptr      = 0x00000020L;
    unsigned int  mask_len = 0x00000000L;

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[]={"mask", "target", "action", "bank", "ptr", "truncate", NULL};
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "s#|bbbIb", kwlist, &mask, &mask_len, &target, &action, &bank, &ptr, &truncate ) )
        return NULL;

    target   = (target & 0x07) << 5;
    action   = (action & 0x07) << 3;
    bank     = (bank   & 0x03) << 0;
    truncate = (truncate == 0 ? 0 : 1);

    length = setSelectParam( target|action|bank, ptr, mask, mask_len, 1, truncate, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_insert( PyObject *self, PyObject *args, PyObject* kws ) {
    unsigned char start  = 0x01;
    unsigned char stop   = 0x05;

    unsigned int  length  = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    static char* kwlist[]={"start", "stop", NULL};
    if ( !PyArg_ParseTupleAndKeywords( args, kws, "|bb", kwlist, &start, &stop ) )
        return NULL;

    if ( stop <= start ) {
        return NULL;
    }

    length = insertRFChannel( start, stop, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_version( PyObject *self, PyObject *args ) {
    unsigned char index  = 0x00;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "|b", &index ) )
        return NULL;

    if ( index > 2 ) {
        return NULL;
    }

    length = version( index, param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_deepsleep( PyObject *self, PyObject *args ) {
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;

    length = deepSleep( param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_scanjammer( PyObject *self, PyObject *args ) {
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "" ) )
        return NULL;

    length = scanJammer( param );
    return Py_BuildValue( "y#", param, length );
}

static PyObject * _m100_deepsleep_time( PyObject *self, PyObject *args ) {
    unsigned char minute = 0x01;
    unsigned int  length = 0x00000000L;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "b", &minute ) )
        return NULL;

    length = deepSleepTime( minute, param );
    return Py_BuildValue( "y#", param, length );
}

/*
* Usage:
*
* def handle_rxpacket( data ):
*     print( data )
*/

static PyObject *cb_unpack_handler = NULL;
static void __callback_event( unsigned char *user_data, unsigned int size )
{
    PyObject *arglist;
    unsigned int  length  = 0;

    if ( NULL == cb_unpack_handler ) {
        return ;
    }

    // Read all data from ringbuffer.
    hexinRingBufferRead( &ringbuffer, cb_buffer, size, &length );

    arglist = Py_BuildValue( "(z#)", cb_buffer, length );
    PyEval_CallObject( (PyObject *)cb_unpack_handler, arglist );
    Py_DECREF( arglist );
}

static PyObject * _m100_unpack( PyObject *self, PyObject *args ) {
    const unsigned char *frame = NULL;
    unsigned int  frame_length = 0x00000000L;
    unsigned int  trigger      = 0xFFFFFFFFL;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE] = { 0x00 };

    if ( !PyArg_ParseTuple( args, "y#|I", &frame, &frame_length, &trigger ) ) {
        return NULL;
    }

    packetHandler( &ringbuffer, trigger, frame, frame_length, __callback_event );

    Py_INCREF(Py_None); /* Boilerplate to return "None" */
    return Py_None;
}

static PyObject * _m100_callback( PyObject *self, PyObject *args ) 
{
    PyObject *handler = NULL;

    if ( !PyArg_ParseTuple(args, "O:set_callback", &handler)) {
        return NULL;
    }
    if ( !PyCallable_Check(handler) ) {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable.");
        return NULL;
    }

    Py_XINCREF( handler );                  /* Add a reference to new callback */
    Py_XDECREF( cb_unpack_handler );        /* Dispose of previous callback */
    cb_unpack_handler = handler;            /* Remember new callback        */
    Py_INCREF(Py_None);                     /* Boilerplate to return "None" */
   
    return Py_None;
}

/* method table */
static PyMethodDef _m100_methods[] = {
    {"idle",            (PyCFunction)_m100_idle,            METH_VARARGS,                   "Module IDLE mode."         },
    {"query",           (PyCFunction)_m100_query,           METH_VARARGS,                   "Query tags."               },
    {"power",           (PyCFunction)_m100_pa_power,        METH_VARARGS,                   "Get or Set RFPower."       },
    {"mode",            (PyCFunction)_m100_reader_mode,     METH_VARARGS,                   "Set reader mode.( MODE_DENSE_READER or MODE_HIGH_SENSITIVITY )"},
    {"stop",            (PyCFunction)_m100_stop,            METH_NOARGS,                    "Stop query."       },
    {"rssi",            (PyCFunction)_m100_rssi,            METH_NOARGS,                    "Test rssi."        },
    {"hfss",            (PyCFunction)_m100_hfss,            METH_VARARGS,                   "HFSS."             },
    {"lock",            (PyCFunction)_m100_lock,            METH_KEYWORDS|METH_VARARGS,     "Lock select tag."  },
    {"kill",            (PyCFunction)_m100_kill,            METH_KEYWORDS|METH_VARARGS,     "Kill select tag."  },
    {"read",            (PyCFunction)_m100_read,            METH_KEYWORDS|METH_VARARGS,     "Read  data from (RFU TID EPC USER)."   },
    {"write",           (PyCFunction)_m100_write,           METH_KEYWORDS|METH_VARARGS,     "Write data to memory bank of user."    },
    {"epc",             (PyCFunction)_m100_write_epc,       METH_KEYWORDS|METH_VARARGS,     "Write data to memory bank of epc."     },
    {"param",           (PyCFunction)_m100_param,           METH_KEYWORDS|METH_VARARGS,     "Set query parameter."  },
    {"select",          (PyCFunction)_m100_select,          METH_KEYWORDS|METH_VARARGS,     "Select tag parameter." },
    {"insert",          (PyCFunction)_m100_insert,          METH_KEYWORDS|METH_VARARGS,     "Insert RF channel."    },
    {"region",          (PyCFunction)_m100_region,          METH_VARARGS,                   "Set module region."    },
    {"unpack",          (PyCFunction)_m100_unpack,          METH_VARARGS,                   "Unpack m100 frame."    },
    {"version",         (PyCFunction)_m100_version,         METH_VARARGS,                   "Get module version information."},
    {"unpack_cb",       (PyCFunction)_m100_callback,        METH_VARARGS,                   "Unpack frame callback."    },
    {"rfcarrier",       (PyCFunction)_m100_rfcarrier,       METH_VARARGS,                   "Turn on/off RF carrier."   },
    {"deepsleep",       (PyCFunction)_m100_deepsleep,       METH_NOARGS,                    "Enter deep sleep."         },
    {"scanjammer",      (PyCFunction)_m100_scanjammer,      METH_NOARGS,                    "Scan jammer."              },
    {"setchannel",      (PyCFunction)_m100_setchannel,      METH_VARARGS,                   "Set RF channel."           },
    {"select_mode",     (PyCFunction)_m100_select_mode,     METH_VARARGS,                   "Set Select mode."          },
    {"demodulator",     (PyCFunction)_m100_demodulator,     METH_KEYWORDS|METH_VARARGS,     "Set receiver demodulator parameter."},
    {"deepsleep_time",  (PyCFunction)_m100_deepsleep_time,  METH_VARARGS,                   "Set deep time time."       },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*******************************************************************************/

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    char antennaGain;

} hexinObject;

static void _m100_dealloc( hexinObject* self )
{
    cb_unpack_handler = NULL;

    PyMem_FREE( rb_buffer );
    PyMem_FREE( cb_buffer );
    rb_buffer = NULL;
    cb_buffer = NULL;

    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject * _m100_new( PyTypeObject *type, PyObject *args, PyObject *kwds )
{
    hexinObject *self;

    self = (hexinObject *)type->tp_alloc( type, 0 );
    // if ( NULL != self ) {
    // }
    return (PyObject *)self;
}

static int _m100_init( hexinObject *self, PyObject *args, PyObject *kwds )
{
    unsigned int  size    = 4096;
    static char *kwlist[] = { "size", NULL };

    if ( !PyArg_ParseTupleAndKeywords( args, kwds, "|IO", kwlist, &size ) ) {
        return -1;
    }

    self->antennaGain = 3;

    rb_buffer = (unsigned char *)PyMem_MALLOC( size );
    cb_buffer = (unsigned char *)PyMem_MALLOC( size );

    if ( (NULL == rb_buffer) || (NULL == cb_buffer) ) {
        return -1;
    }

    if ( hexinRingBufferInit( &ringbuffer, rb_buffer, size ) == HEXIN_FALSE ) {
        return -1;
    }
   
    return 0;
}

static PyMemberDef _m100_members[] = {
    { "ANTENNAGAIN",      T_BYTE,    offsetof( hexinObject, antennaGain),     0,   "Antenna Gain"},
    { NULL }
};

/* module documentation */
PyDoc_STRVAR(_m100_doc,
"\n"
"\n");

PyTypeObject _m100Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_m100.Protocol",          /* tp_name */
    sizeof(hexinObject),       /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)_m100_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    _m100_doc,                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    _m100_methods,             /* tp_methods */
    _m100_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)_m100_init,      /* tp_init */
    0,                         /* tp_alloc */
    _m100_new,                 /* tp_new */
};

/* module definition structure */
static struct PyModuleDef _m100module = {
    PyModuleDef_HEAD_INIT,
    "_m100",             /* name of module */
    _m100_doc,           /* module documentation, may be NULL */
    -1,                  /* size of per-interpreter state of the module */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/* initialization function for Python 3 */
PyMODINIT_FUNC
PyInit__m100(void)
{
    PyObject *mod;

    if ( ( PyType_Ready( &_m100Type ) < 0) ) {
        return NULL;
    }

    mod = PyModule_Create( &_m100module );
    if ( NULL == mod ) {
        return NULL;
    }

    Py_INCREF( &_m100Type );
    PyModule_AddObject( mod, "Protocol", (PyObject *)&_m100Type );

    PyModule_AddStringConstant( mod, "__version__",     "1.2"   );
    PyModule_AddStringConstant( mod, "__author__",      "Heyn"  );

    PyModule_AddIntConstant( mod, "VERSION_HW",               0 );
    PyModule_AddIntConstant( mod, "VERSION_SW",               1 );
    PyModule_AddIntConstant( mod, "VERSION_MFG",              2 );

    PyModule_AddIntConstant( mod, "MODE_DENSE_READER",        1 );
    PyModule_AddIntConstant( mod, "MODE_HIGH_SENSITIVITY",    0 );

    PyModule_AddIntConstant( mod, "HFSS_AUTO",             0xFF );
    PyModule_AddIntConstant( mod, "HFSS_STOP",                0 );

    PyModule_AddIntConstant( mod, "BANK_RFU",          BANK_RFU );
    PyModule_AddIntConstant( mod, "BANK_EPC",          BANK_EPC );
    PyModule_AddIntConstant( mod, "BANK_TID",          BANK_TID );
    PyModule_AddIntConstant( mod, "BANK_USER",        BANK_USER );

    PyModule_AddIntConstant( mod, "PARAM_TARGET_A",           0 );
    PyModule_AddIntConstant( mod, "PARAM_TARGET_B",           1 );

    PyModule_AddIntConstant( mod, "PARAM_SELECT_ALL",         0 );
    PyModule_AddIntConstant( mod, "PARAM_SELECT_NSL",         2 );
    PyModule_AddIntConstant( mod, "PARAM_SELECT_SL",          3 );

    PyModule_AddIntConstant( mod, "PARAM_SESSION_S0",         0 );
    PyModule_AddIntConstant( mod, "PARAM_SESSION_S1",         1 );
    PyModule_AddIntConstant( mod, "PARAM_SESSION_S2",         2 );
    PyModule_AddIntConstant( mod, "PARAM_SESSION_S3",         3 );

    PyModule_AddIntConstant( mod, "TRIGGER_INFO",            HEXIN_MAGICRF_INFO      );
    PyModule_AddIntConstant( mod, "TRIGGER_QUERY",           HEXIN_MAGICRF_QUERY     );
    PyModule_AddIntConstant( mod, "TRIGGER_STOP",            HEXIN_MAGICRF_STOP      );
    PyModule_AddIntConstant( mod, "TRIGGER_ERROR",           HEXIN_MAGICRF_ERROR     );
    PyModule_AddIntConstant( mod, "TRIGGER_READ_DATA",       HEXIN_MAGICRF_READ_DATA );
    PyModule_AddIntConstant( mod, "TRIGGER_RF_CHANNEL",      HEXIN_MAGICRF_GET_RF_CHANNEL    );
    PyModule_AddIntConstant( mod, "TRIGGER_GET_PA_POWER",    HEXIN_MAGICRF_GET_RF_POWER      );
    PyModule_AddIntConstant( mod, "TRIGGER_SET_PA_POWER",    HEXIN_MAGICRF_SET_RF_POWER      );

    PyModule_AddIntMacro( mod, REGION_CHINA_900  );
    PyModule_AddIntMacro( mod, REGION_AMERICA    );
    PyModule_AddIntMacro( mod, REGION_EUROPE     );
    PyModule_AddIntMacro( mod, REGION_CHINA_800  );
    PyModule_AddIntMacro( mod, REGION_KOREA      );

    PyModule_AddIntMacro( mod, MIXER_GAIN_0DB    );
    PyModule_AddIntMacro( mod, MIXER_GAIN_3DB    );
    PyModule_AddIntMacro( mod, MIXER_GAIN_6DB    );
    PyModule_AddIntMacro( mod, MIXER_GAIN_9DB    );
    PyModule_AddIntMacro( mod, MIXER_GAIN_12DB   );
    PyModule_AddIntMacro( mod, MIXER_GAIN_15DB   );
    PyModule_AddIntMacro( mod, MIXER_GAIN_16DB   );

    PyModule_AddIntMacro( mod, IF_GAIN_12DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_18DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_21DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_24DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_27DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_30DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_36DB      );
    PyModule_AddIntMacro( mod, IF_GAIN_40DB      );

    return mod;
}
