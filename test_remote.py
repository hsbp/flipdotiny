#!/usr/bin/env python

from StringIO import StringIO
import unittest
import remote


class TestRemote(unittest.TestCase):
    def setUp(self):
        self.remote = remote.Remote(serial_class=MockSerial)

    def test_init(self):
        self.assertEqual(
                self.get_serial_buf().getvalue(),
                ''.join((chr(0x20 | col) + '\xFF\xFF') for col in xrange(remote.COLS)))
        for pixel in remote.PIXELS:
			self.assertTrue(self.remote.get_pixel(pixel))

    def test_first_block(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.set_pixel((0, 0), False)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\x10\xff\xff\xfe')

    def test_second_block(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.set_pixel((0, 1), False)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\x11\xff\xff\xfe')
        sb.truncate(0)
        self.remote.set_pixel((1, 0), False)
        self.remote.flush_pixels()
        self.assertEqual(sb.getvalue(), '\x10\xff\xff\xfd')

    def test_empty_flush(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        self.remote.flush_pixels()
        self.assertEquals(sb.getvalue(), '')
		
    def test_full(self):
        sb = self.get_serial_buf()
        sb.truncate(0)
        for pixel in remote.PIXELS:
			self.remote.set_pixel(pixel, False)
        for pixel in remote.PIXELS:
			self.assertFalse(self.remote.get_pixel(pixel))
        self.remote.flush_pixels()
        self.assertEqual(
                sb.getvalue(),
                ''.join((chr(0x10 | row) + '\xf0\0\0') for row in xrange(remote.ROWS)))

    def get_serial_buf(self):
        return self.remote.port.dut2tc


class MockSerial(object):
    def __init__(self, port, speed, timeout):
        self.dut2tc = StringIO()
        self.tried_read = False

    def write(self, data):
        self.dut2tc.write(data)

    def read(self):
        if self.tried_read:
            self.tried_read = False
            return 'B'
        else:
            self.tried_read = True
            return ''


if __name__ == '__main__':
    unittest.main()
