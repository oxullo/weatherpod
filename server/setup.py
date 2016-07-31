#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2016 Archimedes Exhibitions GmbH,
# Saarbrücker Str. 24, Berlin, Germany
#
# This file contains proprietary source code and confidential
# information. Its contents may not be disclosed or distributed to
# third parties unless prior specific permission by Archimedes
# Exhibitions GmbH, Berlin, Germany is obtained in writing. This applies
# to copies made in any form and using any medium. It applies to
# partial as well as complete copies.

from __future__ import print_function

from setuptools import find_packages
from setuptools import setup


setup(
    name='weatherpod',
    packages=find_packages(exclude=[]),
    include_package_data=True,
    install_requires=[
        'Pillow',
        'python-forecastio',
        'requests',
        'Flask',
    ],
    tests_require=[
    ],
    entry_points={
        'console_scripts': [
            'weatherpod-server = weatherpod.runner:server',
            'weatherpod-upload = weatherpod.runner:serial_upload'
        ],
    },
    zip_safe=False,
)
