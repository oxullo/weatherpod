#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import logging
import StringIO

from flask import Flask, send_file, request, abort

import forecast
import renderer
import epd

logger = logging.getLogger(__name__)


class Server(object):
    FORMAT_GFX = 0
    FORMAT_EPD = 1

    def __init__(self, config):
        self._config = config
        self._allowed_ids = config.get('server', 'allowed_ids').split(',')

        self._app = Flask(__name__)
        self._renderer = renderer.Renderer(config)
        self._app.add_url_rule('/v1/test/bitmap', view_func=self._test_bitmap)
        self._app.add_url_rule('/v1/test/text', view_func=self._test_text)
        self._app.add_url_rule('/v1/forecast', view_func=self._forecast, methods=['GET', 'POST'])

    def run(self):
        self._app.run(host=self._config.get('server', 'listen_address'),
                      port=self._config.getint('server', 'listen_port'),
                      use_reloader=self._config.getboolean('server', 'use_reloader'))

    def get_app(self):
        return self._app

    def _test_bitmap(self):
        return self._send_image(self._renderer.test_bitmap())

    def _test_text(self):
        return self._send_image(self._renderer.test_text())

    def _forecast(self):
        self._check_authid()

        forecast_instance = forecast.Forecast(self._config)
        current = forecast_instance.retrieve().currently()

        local_data = {'t': self._fetch_number('t'),
                      'p': self._fetch_number('p'),
                      'h': self._fetch_number('h'),
                      'bl': self._fetch_number('bl')}

        return self._send_image(self._renderer.forecast(current, local_data))

    def _get_format(self):
        return self.FORMAT_GFX if request.values.get('fmt') == 'gfx' else self.FORMAT_EPD

    def _fetch_number(self, tag):
        value = request.values.get(tag)

        if value is None:
            return None
        else:
            return int(float(value))

    def _stream_image(self, im):
        sio = StringIO.StringIO()
        im.save(sio, 'PNG')
        sio.seek(0)

        return sio

    def _send_image(self, im):
        if self._get_format() == self.FORMAT_EPD:
            sio = epd.convert(im)
            return send_file(sio, mimetype='image/epd')
        else:
            sio = self._stream_image(im)
            return send_file(sio, mimetype='image/png')

    def _check_authid(self):
        if 'id' not in request.values or request.values['id'] not in self._allowed_ids:
            abort(403)
