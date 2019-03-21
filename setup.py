# -*- coding:utf-8 -*-
""" Setup script for UHF RFID Soc Reader Chip M100 and QM100. """
from os import path
from setuptools import setup, find_packages, Extension
from codecs import open

# !/usr/bin/python
# Python:   3.6.5
# Platform: Windows/Linux/ARMv7
# Author:   Heyn (heyunhuan@gmail.com)
# Program:  Library for UHF RFID Soc Reader Chip M100 and QM100.
# History:  2018-08-15 Wheel Ver:1.0.0 [Heyn] Initialize
#           2018-11-26 Wheel Ver:1.0.1 [Heyn] Optimized cod.

"""
See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/pypa/sampleproject
"""

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='UTF-8') as f:
    long_description = f.read()
    long_description = long_description.replace("\r", "")

setup(
    name='magicrf',
    version='1.2',

    description='Library for UHF RFID Soc Reader Chip M100 and QM100',
    long_description=long_description,

    url='https://github.com/hex-in/libmagicrf.git',

    author='Heyn',
    author_email='heyunhuan@gmail.com',

    license='MIT',

    platforms='any',

    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 3 - Alpha',

        # Indicate who your project is intended for
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Build Tools',

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: MIT License',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: Implementation :: CPython',
        'Programming Language :: Python :: Implementation :: PyPy',
    ],

    # What does your project relate to?
    keywords=['UHF', 'M100', 'QM100', 'rfid', 'magicrf'],

    packages=['magicrf'],

    # List run-time dependencies here.  These will be installed by pip when
    # your project is installed. For an analysis of "install_requires" vs pip's
    # requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires=['pyserial >= 3.4'],

    ext_modules=[Extension('magicrf._m100', sources=['src/_m100.c', 'src/lib/_protocol.c']),
                ],
)
