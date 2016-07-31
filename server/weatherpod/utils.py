#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import time
import pickle


class cached(object):
    def __init__(self, cache_file, timeout):
        self._timeout = timeout
        self._cache_file = cache_file

    def __call__(self, f):
        def decorated():
            if os.path.exists(self._cache_file):
                try:
                    ts, data = pickle.load(open(self._cache_file))
                    if time.time() - ts > self._timeout:
                        data = f()
                except Exception:
                    data = f()
            else:
                data = f()

            pickle.dump([time.time(), data], open(self._cache_file, 'w'))

            return data

        return decorated


def resolve_data_dir(path):
    if os.path.isabs(path):
        return path
    else:
        return os.path.join(os.path.abspath(os.path.dirname(__file__)), 'data', path)
