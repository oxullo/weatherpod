#!/usr/bin/env python

'''
Converts a 400x300px image to an EPD type 0 file (suitable for the MPicoSYS TC2-P441-231 module)
'''

from __future__ import print_function

import sys
import os
import argparse
from PIL import Image

EPD_HEADER = [0x33, 0x01, 0x90, 0x01, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]


def parse_args():
    parser = argparse.ArgumentParser(description='Convert images to EPD type 0 bitmaps')
    parser.add_argument('-o', '--overwrite', action='store_true', default=False, help='Overwrite destination file')
    parser.add_argument('infile', help='Input file (any graphic file format)')
    parser.add_argument('outfile', help='Output file (EPD)')

    return parser.parse_args()

def fail(message):
    print('ERROR: %s' % message, file=sys.stderr)
    sys.exit(1)

def main():
    args = parse_args()

    if os.path.exists(args.outfile) and not args.overwrite:
        fail('Destination file exists. Use --overwrite to skip this check')

    im = Image.open(args.infile)

    if im.size != (400, 300):
        fail('The original size must be 400x300')

    im = im.convert('1')

    fout = open(args.outfile, 'wb')
    fout.write(''.join([chr(c) for c in EPD_HEADER]))

    # EPD pixel data format type 0
    for stride in xrange(im.size[0] * im.size[1] / 8):
        x = (stride * 8) % im.size[0]
        y = (stride * 8) / im.size[0]

        val = 0
        for offs in xrange(8):
            if im.getpixel((x + offs, y)) == 0:
                val |= 1 << (7 - offs)

        fout.write(chr(val))

    fout.close()

if __name__ == '__main__':
    main()
