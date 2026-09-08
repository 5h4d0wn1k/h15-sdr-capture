# SDR Signal Capture Firmware

## Purpose

Study baseband IQ capture, AM/FM demodulation math, and spectrum statistics offline from fixtures.

## Board

- **Board**: ESP32 (I2S ADC IQ front-end)
- **FQBN**: `esp32:esp32:esp32`
- **Sketch**: `h15_sdr_capture/h15_sdr_capture.ino`

## Wiring

```
I channel ADC -> GPIO36, Q channel ADC -> GPIO39, GND common. Use only wired/attenuated bench inputs.
```

## Build

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h15_sdr_capture
# upload (example, ESP32-C6):
# arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyACM0 firmware/h15_sdr_capture
```

## Runtime

See the root README "IMPORTANT" section before powering on. This firmware is
for authorized own-lab study. Serial console exposes the interactive command
set described in the root README. All identifiers in the sketch are
placeholders (`lab-*` SSIDs, `00:11:22:33:44:55`, RFC 5737 / example.com).
