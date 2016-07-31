#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import StringIO

EPD_HEADER = [0x33, 0x01, 0x90, 0x01, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

def test_canvas():
    data = [0xaa] * 15000
    sio = StringIO.StringIO()
    sio.write(''.join([chr(c) for c in EPD_HEADER + data]))
    sio.seek(0)

    return sio

def convert(im):
    assert im.size == (400, 300)
    sio = StringIO.StringIO()
    sio.write(''.join([chr(c) for c in EPD_HEADER]))

    # EPD pixel data format type 0
    for stride in xrange(im.size[0] * im.size[1] / 8):
        x = (stride * 8) % im.size[0]
        y = (stride * 8) / im.size[0]

        val = 0
        for offs in xrange(8):
            if im.getpixel((x + offs, y)) == 0:
                val |= 1 << (7 - offs)

        sio.write(chr(val))

    sio.seek(0)

    return sio
