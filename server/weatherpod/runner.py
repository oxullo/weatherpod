#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import argparse

def server():
    parser = argparse.ArgumentParser()
    parser.add_argument('config')

    args = parser.parse_args()

    os.environ['WEATHERPOD_SETTINGS'] = os.path.abspath(args.config)

    import server
    server.app.run(host='0.0.0.0', use_reloader=True)

def serial_upload():
    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('serialport')

    args = parser.parse_args()

    import serial_uploader

    serial_uploader.upload(args.infile, args.serialport)
