#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import time
import pickle

import forecastio


class Forecast(object):
    def __init__(self, config):
        self.api_key = config['forecastio_api_key']
        self.coordinates = config['coordinates']
        self.units = config['units']
        self.cache_file = config['cache_file']
        self.cache_lifetime = config['cache_lifetime']

    def retrieve(self):
        if os.path.exists(self.cache_file):
            try:
                ts, data = pickle.load(open(self.cache_file))
                if time.time() - ts > self.cache_lifetime:
                    data = self._retrieve_from_net()
            except Exception:
                data = self._retrieve_from_net()
        else:
            data = self._retrieve_from_net()

        pickle.dump([time.time(), data], open(self.cache_file, 'w'))

        return data

    def _retrieve_from_net(self):
        print('Retrieving forecast for coordinates: %s' % (self.coordinates,))
        forecast = forecastio.load_forecast(self.api_key, self.coordinates[0], self.coordinates[1], units=self.units)

        return forecast
