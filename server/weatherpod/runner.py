#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import sys
import os
import argparse
import ConfigParser
import logging

logger = logging.getLogger(__name__)

def server():
    parser = argparse.ArgumentParser()
    parser.add_argument('config')
    parser.add_argument('--debug', '-d', action='store_true')

    args = parser.parse_args()

    logging.basicConfig(level=logging.DEBUG if args.debug else logging.INFO)

    if not os.path.exists(args.config):
        logger.error('Cannot find config file %s' % args.config)
        sys.exit(1)

    config = ConfigParser.ConfigParser()
    config.read(args.config)

    import server
    server_instance = server.Server(config)
    server_instance.run()

def serial_upload():
    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('serialport')

    args = parser.parse_args()

    import serial_uploader

    serial_uploader.upload(args.infile, args.serialport)
