#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Uploads a picture to the TCM module via serial.
Note: this mode requires the firmware serial_streamer to be flashed first
'''

from __future__ import print_function

import serial
from PIL import Image

import epd

def upload(input_file, serial_port):
    im = Image.open(input_file)

    if im.size != (400, 300):
        print('WARNING: resizing image')

    im = im.resize((400, 300))
    im = im.convert('1')

    s = serial.Serial(serial_port, 115200, timeout=5)
    sio = epd.convert(im)

    s.flushInput()

    written = 0
    for c in sio:
        written += s.write(c)

    print('%d bytes written' % written)

    rc = s.read(3)

    if rc == 'ACK':
        print('Image correctly transferred')
    else:
        print('ERROR: %s' % rc)
