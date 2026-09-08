#!/usr/bin/env python3
"""H15 - SDR Capture host helper: offline IQ power/demod analysis.
Educational/authorized own-lab use only (see README "IMPORTANT").
"""
import argparse, math, os, sys

MOD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, MOD)
from hw_common import DEMO_TAG, read_target


def parse_iq(text):
    iq = []
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) != 2:
            continue
        try:
            iq.append((float(parts[0]), float(parts[1])))
        except ValueError:
            continue
    return iq


def power(iq):
    if not iq:
        return -100.0
    import math
    rms = math.sqrt(sum(i * i + q * q for i, q in iq) / len(iq))
    return 20.0 * math.log10(rms + 1e-10)


def am_envelope(iq):
    return [math.hypot(i, q) for i, q in iq]


def analyze(text):
    iq = parse_iq(text)
    return {"count": len(iq), "power_db": power(iq)}


def run_demo():
    print("=== H15 IQ power analysis (offline) ===")
    text = read_target("fixtures/iq_samples.txt",
                       "\n".join("1000 0" for _ in range(16)) + "\n")
    a = analyze(text)
    print("  samples: %d  power: %.1f dB" % (a["count"], a["power_db"]))
    print(DEMO_TAG)
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(
        description="H15 SDR capture - offline IQ power analysis")
    p.add_argument("--demo", action="store_true", help="offline demo (exit 0)")
    p.add_argument("--file", help="IQ sample file")
    args = p.parse_args(argv)
    text = read_target("fixtures/iq_samples.txt")
    if args.file:
        text = open(args.file).read()
    if args.demo or not args.file:
        return run_demo()
    print(analyze(text))
    return 0


if __name__ == "__main__":
    sys.exit(main())
