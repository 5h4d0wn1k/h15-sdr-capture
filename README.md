# H15 — SDR Signal Capture

SDR signal processing, IQ data capture, and FM/AM demodulation for ESP32.

## Overview

Software-defined radio toolkit using ESP32's ADC:
- IQ baseband capture via dual ADC channels
- AM demodulation (envelope detection)
- FM demodulation (arctangent discriminator)
- Power spectrum computation
- Binary IQ data export for external analysis

## Hardware

| Component | Connection | Role |
|-----------|------------|------|
| ESP32 DevKit | Main board | ADC + processing |
| IQ input I | GPIO36 (ADC1_CH0) | In-phase signal |
| IQ input Q | GPIO39 (ADC1_CH3) | Quadrature signal |

## Features

- **IQ capture**: 12-bit dual ADC at ~200 kSps
- **AM demod**: Envelope detection with DC removal and normalization
- **FM demod**: Cross-product discriminator with moving average filter
- **Spectrum**: ASCII bar graph of frequency bins
- **Signal stats**: Power (dB), peak frequency offset, SNR estimation
- **Binary export**: Raw IQ data for GNU Octave / Python analysis

## Serial Output

```
=== H15 — SDR Signal Capture ===
IQ capture: I=GPIO36, Q=GPIO39

── Spectrum ──
  [  0] ########                              0.125
  [  8] ################################      0.842
  [ 16] ##########                            0.215
  [ 24] ###                                   0.078

Power:   -12.3 dB
Peak:    1200 Hz offset
```

## Build & Flash

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 firmware/
```

## Legal Disclaimer

**IMPORTANT: Read before use.**

This project is provided for **educational and authorized security testing purposes only**.

### Authorization Requirements
- You MUST have explicit written permission from the network owner before using this tool
- Unauthorized interception of network communications is illegal under federal and state laws
- This tool should ONLY be used on networks you own or have written authorization to test

### Legal Framework
- **Computer Fraud and Abuse Act (CFAA)**: Unauthorized access to computer systems is a federal crime
- **Wiretap Act (18 U.S.C. § 2511)**: Interception of electronic communications without consent is illegal
- **State Laws**: Many states have additional computer crime and wiretapping statutes
- **GDPR/CCPA**: Data collection may be subject to privacy regulations

### Acceptable Use
- Testing security of your own networks
- Authorized penetration testing with written scope
- Academic research in controlled lab environments
- Security education and training

### Prohibited Use
- Intercepting communications on networks you don't own
- Attacking infrastructure without authorization
- Any activity that violates applicable laws or regulations
- Commercial use without proper licensing

### No Warranty
This software is provided "AS IS" without warranty of any kind. The author is not responsible for any misuse or damage caused by this software.

### Responsible Disclosure
If you discover vulnerabilities using this tool, follow responsible disclosure practices:
1. Report to the vendor/owner privately
2. Allow reasonable time for remediation
3. Do not exploit beyond proof of concept

## License

MIT
