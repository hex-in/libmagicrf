MagicRF
=======

MagicRF is a library for M100 and QM100 modules.


Installation
------------

* Compile and install the library::

    pip3 install magicrf
    
* After installation you can run unit tests to make sure that the library works fine.  Execute::

    python -m magicrf.case1

Usage
-----

  In Python 3::
    from magicrf import m100

    reader = m100('COM14')
    
    def receive_callback(data):
        for item in data[:-1].split(';'):
            epc, rssi = item.split(',')
            print('{0} RSSI: -{1} dBm'.format(epc, int(rssi, 16)))
    
    reader.rxcallback( receive_callback )
    reader.start()

    # reader.power(22)
    # reader.param(q=4)
    reader.query(100)

Example
-------

    from magicrf import m100

    # Get PA Power
    m100.power()
    # Set PA Power
    m100.power(22.0)


V1.0.1 (2018-11-26)
+++++++++++++++++++
* Release ver1.0.1


V1.0 (2018-08-15)
+++++++++++++++++++
* Initialization

V1.1 (2018-10-24)
+++++++++++++++++++
* New query\power etc. functions.
