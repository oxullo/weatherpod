#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os
import requests

from PIL import Image, ImageDraw, ImageFont

from utils import resolve_data_dir

class Renderer(object):
    def __init__(self, config):
        self.config = config
        self.font = resolve_data_dir(config['font'])
        self.icons_path = resolve_data_dir(config['icons_path'])

    def simple(self, forecast_data):
        im = Image.new('1', (400, 300), (255,))
        icon = Image.open(os.path.join(resolve_data_dir(self.icons_path), '%s.png') % forecast_data.icon)
        icon.convert('1')
        im.paste(icon, (220, 120))

        draw = ImageDraw.Draw(im)
        font_big = ImageFont.truetype(self.font, 60)
        font_small = ImageFont.truetype(self.font, 20)
        draw.text((20, 10), '%dC' % forecast_data.temperature, font=font_big)
        draw.text((20, 80), '%d%% %dmbar pp %d%%' % (forecast_data.humidity * 100, forecast_data.pressure,
                                                     forecast_data.precipProbability*100), font=font_small)
        draw.text((20, 105), forecast_data.summary, font=font_small)

        return im

    def randompic(self):
        r = requests.get(self.config['random_pic_provider_url'], stream=True)
        if r.status_code == 200:
            with open('out.jpg', 'wb') as f:
                for chunk in r:
                    f.write(chunk)

        im = Image.open('out.jpg')
        return im.convert('1')
