#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import datetime
from PIL import Image, ImageDraw, ImageFont

from utils import resolve_data_dir

class Renderer(object):
    def __init__(self, config):
        self._config = config
        self._font = resolve_data_dir(config.get('renderer', 'font'))
        self._icons_path = resolve_data_dir(config.get('renderer', 'icons_path'))
        self._template = resolve_data_dir(config.get('renderer', 'template'))
        self._test_bitmap = resolve_data_dir(config.get('renderer', 'test_bitmap'))

    def forecast(self, outside_t, min_t, max_t, inside_t, precip_pct, battery_mv, words):
        im = Image.new('1', size=(400, 300), color=255)

        draw = ImageDraw.Draw(im)
        font_big = ImageFont.truetype(self._font, 64)
        font_half = ImageFont.truetype(self._font, 28)
        font_small = ImageFont.truetype(self._font, 12)

        draw.text((20, 10), u'%dºC - %dºC' % (min_t, max_t), font=font_big)
        draw.text((20, 80), u'E:%dºC / I:%dºC PP:%d%%' % (outside_t, inside_t, (precip_pct * 100)), font=font_half)

        draw.text((20, 160), ', '.join(words), font=font_half)

        draw.text((20, 266), u'BL=%smV RTS=%s' % (battery_mv, datetime.datetime.now()), font=font_small)

        return im

    def test_bitmap(self):
        im = Image.open(self._test_bitmap)

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
