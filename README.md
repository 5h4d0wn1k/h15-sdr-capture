> **⚠️ EDUCATIONAL USE ONLY — AUTHORIZED TESTING ONLY.**
> This project exists for education, research, and **defense of systems you own
> or hold explicit written authorization to assess**. Unauthorized use is
> prohibited and may be illegal. Read [ETHICS.md](ETHICS.md) and
> [SCOPE.md](SCOPE.md) before use. Use at your own risk; **AS IS**, no warranty.

# H15 — SDR Signal Capture & RF Forensics

Software-defined radio toolkit: IQ baseband capture via ESP32 dual-ADC channels, AM/FM demodulation, power spectrum, signal statistics, and binary IQ export — with an offline host analysis CLI.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/5h4d0wn1k/h15-sdr-capture.svg)](https://github.com/5h4d0wn1k/h15-sdr-capture)
[![Last commit](https://img.shields.io/github/last-commit/5h4d0wn1k/h15-sdr-capture.svg)](https://github.com/5h4d0wn1k/h15-sdr-capture)
[![Issues](https://img.shields.io/github/issues/5h4d0wn1k/h15-sdr-capture.svg)](https://github.com/5h4d0wn1k/h15-sdr-capture)

## Why

RF is a rich source of security signal — if you can demodulate it. H15 turns an ESP32's ADC into an IQ sampling front end (~200 kSps, 12-bit, channels I and Q), implements real AM (envelope) and FM (cross-product discriminator) demodulators, computes power spectra and statistics, and exports raw IQ for deeper analysis in GNU Octave or Python. All host-side analysis runs offline against bundled fixtures, so the DSP skills carry over to any hardware you own.

## Features

- **IQ baseband capture** — dual ADC channels (I=`GPIO36` ADC1_CH0, Q=`GPIO39` ADC1_CH3)
- **AM demodulation** — envelope detection with DC removal and normalization
- **FM demodulation** — arctangent cross-product discriminator with moving-average filter
- **Power spectrum** — ASCII bar graph of frequency bins, power (dB), peak frequency offset, SNR estimate
- **Binary IQ export** — raw samples for GNU Octave / Python analysis
- **Offline host CLI** — `host/h15_cli.py --demo` analyzes bundled `fixtures/iq_samples.txt`

## Quickstart

```bash
# Offline host analysis (no hardware needed)
python3 host/h15_cli.py --demo

# Analyze your own IQ sample file
python3 host/h15_cli.py --file my_samples.txt

# Unit tests
python3 -m unittest discover -s tests

# Compile the ESP32 firmware (Arduino CLI)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/
```

## Hardware map

| Component | Connection | Role |
|---|---|---|
| ESP32 DevKit | Main board | ADC + DSP |
| IQ input I | GPIO36 (ADC1_CH0) | In-phase signal |
| IQ input Q | GPIO39 (ADC1_CH3) | Quadrature signal |

## Project structure

- `firmware/h15_sdr_capture/` — ESP32 capture + demodulation firmware
- `host/h15_cli.py` — offline IQ analysis CLI; `host/hw_common.py` — shared helpers
- `fixtures/iq_samples.txt` — bundled sample for the demo; `tests/` — unit tests

## Documentation

- [ETHICS.md](ETHICS.md) — educational purpose and authorized use only
- [SCOPE.md](SCOPE.md) — authorized-testing scope checklist
- [SECURITY.md](SECURITY.md) — vulnerability reporting
- [CONTRIBUTING.md](CONTRIBUTING.md) — safe contribution guidelines

## Contributing

New demodulators, spectrum features and fixture coverage are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md); lab work must stay on spectrum and devices you own.

## License

MIT — see [LICENSE](LICENSE). Provided **AS IS**, without warranty, for education and authorized RF / hardware security lab use only.