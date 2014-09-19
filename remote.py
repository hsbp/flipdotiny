#!/usr/bin/env python

from serial import Serial
from itertools import product
from collections import defaultdict
import struct

COLS = 20
ROWS = 16
PIXELS = set(product(xrange(COLS), xrange(ROWS)))

class Remote(object):
    BLACK = True
    WHITE = False
    COLORS = (BLACK, WHITE)

    def __init__(self, serial_class=Serial, port='/dev/ttyUSB0'):
        self.port = serial_class(port, 9600, timeout=0.1)
        self.framebuf = [None] * ROWS * COLS
        self.clip = {color: defaultdict(int) for color in self.COLORS}
        for pixel in PIXELS:
            self.set_pixel(pixel, self.BLACK)
        self.flush_pixels()

    def get_pixel(self, pixel):
        return self.framebuf[pixel2fbindex(pixel)]

    def set_pixel(self, pixel, value):
        fbi = pixel2fbindex(pixel)
        if self.framebuf[fbi] == value:
            return
        self.framebuf[fbi] = value
        self.clip[value][pixel[int(not value)]] |= 1 << pixel[int(value)]

    def flush_pixels(self):
        for color, lines in self.clip.items():
            if not lines:
                continue
            for num, line in lines.items():
                if color == self.BLACK:
                    self.port.write(chr(0x20 | num) + struct.pack('>H', line))
                elif color == self.WHITE:
                    self.port.write(chr(0x10 | num) + struct.pack('>I', line ^ 0xFFFFFF)[1:])
                else:
                    raise ValueError('Invalid color {0}'.format(color))
                while not self.port.read():
                    pass
            lines.clear()

def pixel2fbindex(pixel):
    col, row = pixel
    return row * COLS + col

if __name__ == '__main__':
    from time import sleep
    r = Remote()
    for y in xrange(ROWS):
        for x in xrange(COLS):
            r.set_pixel((x, y), False)
            r.flush_pixels()
            sleep(0.05)
            r.set_pixel((x, y), True)
