#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import time
import pickle

import forecastio


class Forecast(object):
    def __init__(self, config):
        self._api_key = config.get('forecastio', 'api_key')
        self._coordinates = [config.getfloat('forecastio', 'latitude'),
                             config.getfloat('forecastio', 'longitude')]
        self._units = config.get('forecastio', 'units')
        self._cache_file = config.get('cache', 'file')
        self._cache_lifetime = config.getfloat('cache', 'lifetime')

    def retrieve(self):
        if os.path.exists(self._cache_file):
            try:
                ts, data = pickle.load(open(self._cache_file))
                if time.time() - ts > self._cache_lifetime:
                    data = self._retrieve_from_net()
            except Exception:
                data = self._retrieve_from_net()
        else:
            data = self._retrieve_from_net()

        pickle.dump([time.time(), data], open(self._cache_file, 'w'))

        return data

    def _retrieve_from_net(self):
        print('Retrieving forecast for coordinates: %s' % (self._coordinates,))
        forecast = forecastio.load_forecast(self._api_key, self._coordinates[0], self._coordinates[1], units=self._units)

        return forecast
