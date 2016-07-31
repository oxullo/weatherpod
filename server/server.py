#!/usr/bin/env python

'''
Canvas dispatcher for the weatherpod unit
Runs on a server and generates the bitmap, in EPD format, that the pod downloads and displays
'''

from __future__ import print_function

import requests
from PIL import Image

from flask import Flask
from flask import send_file
from flask import request
import StringIO

EPD_HEADER = [0x33, 0x01, 0x90, 0x01, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

app = Flask(__name__)

def test_canvas():
    data = [0xaa] * 15000
    sio = StringIO.StringIO()
    sio.write(''.join([chr(c) for c in EPD_HEADER + data]))
    sio.seek(0)

    return sio


@app.route('/weatherpod/v1/testpattern')
def request_canvas():
    return send_file(test_canvas(), mimetype='image/edp')

@app.route('/weatherpod/v1/testbitmap', methods=['GET', 'POST'])
def testbitmap():
    print(request.values)
    r = requests.get('http://lorempixel.com/g/400/300/', stream=True)
    if r.status_code == 200:
        with open('out.jpg', 'wb') as f:
            for chunk in r:
                f.write(chunk)

    im = Image.open('out.jpg')
    im = im.convert('1')
    im.save('bitmap.png')

    print('Image size:', im.size)

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

    return send_file(sio, mimetype='image/edp')


if __name__ == '__main__':
    app.run(host='0.0.0.0', use_reloader=True)
