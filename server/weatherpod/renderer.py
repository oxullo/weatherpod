#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import io
import requests

from PIL import Image, ImageDraw, ImageFont

from utils import resolve_data_dir

class Renderer(object):
    def __init__(self, config):
        self._config = config
        self._font = resolve_data_dir(config.get('renderer', 'font'))
        self._icons_path = resolve_data_dir(config.get('renderer', 'icons_path'))

    def forecast(self, forecast_data):
        im = Image.new('1', (400, 300), (255,))
        icon = Image.open(os.path.join(resolve_data_dir(self._icons_path), '%s.png') % forecast_data.icon)
        icon.convert('1')
        im.paste(icon, (220, 120))

        draw = ImageDraw.Draw(im)
        font_big = ImageFont.truetype(self._font, 60)
        font_small = ImageFont.truetype(self._font, 20)
        draw.text((20, 10), '%dC' % forecast_data.temperature, font=font_big)
        draw.text((20, 80), '%d%% %dmbar pp %d%%' % (forecast_data.humidity * 100, forecast_data.pressure,
                                                     forecast_data.precipProbability*100), font=font_small)
        draw.text((20, 105), forecast_data.summary, font=font_small)

        return im

    def test_bitmap(self):
        im = Image.open(resolve_data_dir('tests/test-image.png'))

        return im.convert('1')

    def test_text(self):
        im = Image.new('1', (1600, 1200), (255,))

        draw = ImageDraw.Draw(im)
        offset = 0
        for height in xrange(4, 40, 10):
            draw.text((0, offset), 'The quick brown fox jumps over the lazy dog',
                      font=ImageFont.truetype(self._font, height * 10))
            offset += height * 10

        return im.resize((400, 300))
