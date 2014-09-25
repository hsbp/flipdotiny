#!/usr/bin/python

from serial import Serial

COLS = 20
ROWS = 16

class FlipDots(object):
    def __init__(self):
        self.port = Serial('/dev/ttyUSB0', 9600)

    def clear_black(self):
        for col in xrange(COLS):
            self.port.write(chr(0x20 | col) + '\xFF' * 2)
            while not self.port.read():
                pass

    def clear_white(self):
        for row in xrange(ROWS):
            self.port.write(chr(0x10 | row) + '\x00' * 3)
            while not self.port.read():
                pass

    def checkboard(self):
        self.clear_black()
        for row in xrange(ROWS):
            self.port.write(chr(0x10 | row) + (chr(0x55) if row % 2 else chr(0xAA)) * 3)
            while not self.port.read():
                pass

if __name__ == '__main__':
    fd = FlipDots()
    fd.checkboard()
