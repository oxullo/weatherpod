#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function


from flask import Flask, send_file, request

import StringIO

import forecast
import renderer
import epd

app = Flask(__name__)
app.config.from_object('weatherpod.default_settings')
app.config.from_envvar('WEATHERPOD_SETTINGS')


def stream_image(im):
    sio = StringIO.StringIO()
    im.save(sio, 'PNG')
    sio.seek(0)

    return sio

@app.route('/weatherpod/v1/testpattern')
def request_canvas():
    return send_file(epd.test_canvas(), mimetype='image/epd')

@app.route('/weatherpod/v1/testbitmap')
def testbitmap():
    im = renderer.randompic()

    sio = epd.convert(im)

    return send_file(sio, mimetype='image/epd')

@app.route('/weatherpod/v1/simple', methods=['GET', 'POST'])
def simple():
    print(request.form)
    forecast_instance = forecast.Forecast(app.config.get_namespace('FORECAST_'))
    current_forecast = forecast_instance.retrieve().currently()

    renderer_instance = renderer.Renderer(app.config.get_namespace('RENDERER_'))
    im = renderer_instance.simple(current_forecast)

    fmt = request.args.get('format')

    if fmt == 'png':
        sio = stream_image(im)
        return send_file(sio, mimetype='image/png')
    else:
        sio = epd.convert(im)
        return send_file(sio, mimetype='image/epd')


if __name__ == '__main__':
    app.run(host='0.0.0.0', use_reloader=True)
