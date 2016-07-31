#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import argparse

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('config')

    args = parser.parse_args()

    os.environ['WEATHERPOD_SETTINGS'] = os.path.abspath(args.config)

    import server
    server.app.run(host='0.0.0.0', use_reloader=True)
