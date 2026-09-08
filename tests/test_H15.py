import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "host"))
import h15_cli as m


class TestIQ(unittest.TestCase):
    def test_parse(self):
        iq = m.parse_iq("1000 0\n-1000 0\n")
        self.assertEqual(len(iq), 2)

    def test_power_positive(self):
        iq = m.parse_iq("2048 0\n2048 0\n")
        db = m.power(iq)
        self.assertGreater(db, -20)

    def test_am_envelope(self):
        iq = m.parse_iq("3000 -4000\n")
        self.assertAlmostEqual(m.am_envelope(iq)[0], 5000.0, places=3)

    def test_empty_power(self):
        self.assertEqual(m.power([]), -100.0)


if __name__ == "__main__":
    unittest.main()
