// H15 — SDR Signal Capture (ESP32)
// SDR signal processing, IQ data capture, FM/AM demodulation
// Uses I2S ADC for baseband IQ sampling

#include <Arduino.h>
#include <driver/i2s.h>

// ── Configuration ──────────────────────────────────────────────
#define SERIAL_BAUD       115200
#define SAMPLE_RATE       204800
#define IQ_BUFFER_SIZE    1024
#define AM_DEMOD_SIZE     512
#define FM_DEMOD_SIZE     512
#define ADC_PIN_I         36   // GPIO36 (ADC1_CH0) — I channel
#define ADC_PIN_Q         39   // GPIO39 (ADC1_CH3) — Q channel
#define I2S_PORT          I2S_NUM_0

// ── I2S config for ADC input ─────────────────────────────────
#define I2S_WS_PIN        25
#define I2S_SCK_PIN       26
#define I2S_SD_PIN        22

// ── IQ sample buffer ─────────────────────────────────────────
int16_t iBuffer[IQ_BUFFER_SIZE];
int16_t qBuffer[IQ_BUFFER_SIZE];

// ── Demodulated output ───────────────────────────────────────
float amOutput[AM_DEMOD_SIZE];
float fmOutput[FM_DEMOD_SIZE];

// ── Signal stats ─────────────────────────────────────────────
struct SignalStats {
  float  power;
  float  peakFreq;
  float  noiseFloor;
  float  snr;
  uint32_t sampleCount;
};
SignalStats stats = {0, 0, -60, 0, 0};

// ── Forward declarations ──────────────────────────────────────
void    initI2S();
void    captureIQ(int16_t* iBuf, int16_t* qBuf, int count);
void    amDemodulate(int16_t* iBuf, int16_t* qBuf, int len, float* out);
void    fmDemodulate(int16_t* iBuf, int16_t* qBuf, int len, float* out);
float   computePower(int16_t* buf, int len);
float   computePeakFreq(int16_t* iBuf, int16_t* qBuf, int len);
void    printSpectrum(float* data, int len);
void    printIQ(const char* label, int16_t* iBuf, int16_t* qBuf, int len);
void    exportIQ();
void    exportDemod(const char* type, float* data, int len);

// ── Setup ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);
  Serial.println(F("\n=== H15 — SDR Signal Capture ==="));

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(ADC_PIN_I, INPUT);
  pinMode(ADC_PIN_Q, INPUT);

  Serial.printf("IQ capture: I=GPIO%d, Q=GPIO%d\n", ADC_PIN_I, ADC_PIN_Q);
  Serial.printf("Sample rate: %d Hz, Buffer: %d samples\n", SAMPLE_RATE, IQ_BUFFER_SIZE);
  Serial.println(F("\nCommands:"));
  Serial.println(F("  c = Capture IQ"));
  Serial.println(F("  a = AM demodulate"));
  Serial.println(F("  f = FM demodulate"));
  Serial.println(F("  p = Power spectrum"));
  Serial.println(F("  e = Export IQ (binary)"));
  Serial.println(F("  s = Signal stats\n"));
}

// ── Main loop ─────────────────────────────────────────────────
void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'c': case 'C':
        captureIQ(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        printIQ("IQ Capture", iBuffer, qBuffer, 32);
        stats.power = computePower(iBuffer, IQ_BUFFER_SIZE);
        stats.sampleCount += IQ_BUFFER_SIZE;
        Serial.printf("Power: %.1f dB\n", stats.power);
        break;

      case 'a': case 'A':
        captureIQ(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        amDemodulate(iBuffer, qBuffer, IQ_BUFFER_SIZE, amOutput);
        printSpectrum(amOutput, AM_DEMOD_SIZE);
        Serial.println("AM demod output (first 32 samples):");
        for (int i = 0; i < min(32, AM_DEMOD_SIZE); i++) {
          Serial.printf("  [%3d] %.2f\n", i, amOutput[i]);
        }
        break;

      case 'f': case 'F':
        captureIQ(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        fmDemodulate(iBuffer, qBuffer, IQ_BUFFER_SIZE, fmOutput);
        printSpectrum(fmOutput, FM_DEMOD_SIZE);
        Serial.println("FM demod output (first 32 samples):");
        for (int i = 0; i < min(32, FM_DEMOD_SIZE); i++) {
          Serial.printf("  [%3d] %.2f\n", i, fmOutput[i]);
        }
        break;

      case 'p': case 'P':
        captureIQ(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        stats.power = computePower(iBuffer, IQ_BUFFER_SIZE);
        stats.peakFreq = computePeakFreq(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        Serial.printf("\n── Signal Power ──\n");
        Serial.printf("Power:   %.1f dB\n", stats.power);
        Serial.printf("Peak:    %.0f Hz offset\n", stats.peakFreq);
        break;

      case 'e': case 'E':
        captureIQ(iBuffer, qBuffer, IQ_BUFFER_SIZE);
        exportIQ();
        break;

      case 's': case 'S':
        Serial.printf("\n── SDR Stats ──\n");
        Serial.printf("Samples:   %lu\n", stats.sampleCount);
        Serial.printf("Power:     %.1f dB\n", stats.power);
        Serial.printf("Peak freq: %.0f Hz\n", stats.peakFreq);
        Serial.printf("Noise:     %.1f dB\n", stats.noiseFloor);
        break;
    }
  }
}

// ── Init I2S for ADC ─────────────────────────────────────────
void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = true,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("I2S ADC initialized");
}

// ── Capture IQ samples ───────────────────────────────────────
void captureIQ(int16_t* iBuf, int16_t* qBuf, int count) {
  uint32_t t0 = micros();

  for (int i = 0; i < count; i++) {
    iBuf[i] = analogRead(ADC_PIN_I) - 2048;  // Center around 0
    qBuf[i] = analogRead(ADC_PIN_Q) - 2048;

    // crude timing for sample rate
    while (micros() - t0 < (uint32_t)i * 1000000UL / SAMPLE_RATE) { }
  }

  uint32_t elapsed = micros() - t0;
  Serial.printf("Captured %d IQ samples in %lu us (%.0f kSps)\n",
    count, elapsed, (float)count / elapsed * 1000000.0);
}

// ── AM Demodulation (envelope detection) ─────────────────────
void amDemodulate(int16_t* iBuf, int16_t* qBuf, int len, float* out) {
  int outLen = min(len, AM_DEMOD_SIZE);

  // Envelope: sqrt(I² + Q²)
  for (int i = 0; i < outLen; i++) {
    float fi = (float)iBuf[i];
    float fq = (float)qBuf[i];
    out[i] = sqrtf(fi * fi + fq * fq);
  }

  // Remove DC offset
  float dcSum = 0;
  for (int i = 0; i < outLen; i++) dcSum += out[i];
  float dc = dcSum / outLen;
  for (int i = 0; i < outLen; i++) out[i] -= dc;

  // Normalize
  float maxVal = 0;
  for (int i = 0; i < outLen; i++) {
    float a = fabsf(out[i]);
    if (a > maxVal) maxVal = a;
  }
  if (maxVal > 0) {
    for (int i = 0; i < outLen; i++) out[i] /= maxVal;
  }
}

// ── FM Demodulation (arctangent discriminator) ───────────────
void fmDemodulate(int16_t* iBuf, int16_t* qBuf, int len, float* out) {
  int outLen = min(len - 1, FM_DEMOD_SIZE);

  for (int i = 0; i < outLen; i++) {
    // Previous sample
    float iPrev = (float)iBuf[i];
    float qPrev = (float)qBuf[i];
    // Current sample
    float iCurr = (float)iBuf[i + 1];
    float qCurr = (float)qBuf[i + 1];

    // Cross product discriminator
    float denom = iCurr * iCurr + qCurr * qCurr;
    if (denom < 1.0f) denom = 1.0f;
    out[i] = (iPrev * qCurr - iCurr * qPrev) / denom;
  }

  // Low-pass filter (simple moving average)
  const int winSize = 4;
  for (int i = winSize; i < outLen; i++) {
    float sum = 0;
    for (int j = 0; j < winSize; j++) sum += out[i - j];
    out[i] = sum / winSize;
  }

  // Normalize
  float maxVal = 0;
  for (int i = 0; i < outLen; i++) {
    float a = fabsf(out[i]);
    if (a > maxVal) maxVal = a;
  }
  if (maxVal > 0) {
    for (int i = 0; i < outLen; i++) out[i] /= maxVal;
  }
}

// ── Compute signal power ─────────────────────────────────────
float computePower(int16_t* buf, int len) {
  float sum = 0;
  for (int i = 0; i < len; i++) {
    float v = (float)buf[i] / 2048.0f;
    sum += v * v;
  }
  float rms = sqrtf(sum / len);
  if (rms < 1e-10f) return -100.0f;
  return 20.0f * log10f(rms);
}

// ── Compute peak frequency offset ────────────────────────────
float computePeakFreq(int16_t* iBuf, int16_t* qBuf, int len) {
  float maxMag = 0;
  int maxBin = 0;

  for (int k = 0; k < len / 2; k++) {
    float sumI = 0, sumQ = 0;
    for (int n = 0; n < len; n++) {
      float angle = 2.0f * PI * k * n / len;
      sumI += (float)iBuf[n] * cosf(angle) + (float)qBuf[n] * sinf(angle);
      sumQ += (float)qBuf[n] * cosf(angle) - (float)iBuf[n] * sinf(angle);
    }
    float mag = sumI * sumI + sumQ * sumQ;
    if (mag > maxMag) {
      maxMag = mag;
      maxBin = k;
    }
  }

  return (float)maxBin * SAMPLE_RATE / len;
}

// ── Print spectrum (ASCII) ───────────────────────────────────
void printSpectrum(float* data, int len) {
  Serial.println("\n── Spectrum ──");
  int bins = 32;
  int binSize = len / bins;

  for (int b = 0; b < bins; b++) {
    float sum = 0;
    for (int i = 0; i < binSize; i++) sum += fabsf(data[b * binSize + i]);
    float avg = sum / binSize;

    int barLen = (int)(avg * 40);
    if (barLen > 40) barLen = 40;
    if (barLen < 0) barLen = 0;

    Serial.printf("  [%3d] ", b * binSize);
    for (int i = 0; i < barLen; i++) Serial.write('#');
    for (int i = barLen; i < 40; i++) Serial.write(' ');
    Serial.printf(" %.3f\n", avg);
  }
}

// ── Print IQ samples ─────────────────────────────────────────
void printIQ(const char* label, int16_t* iBuf, int16_t* qBuf, int len) {
  Serial.printf("\n── %s (first %d samples) ──\n", label, len);
  for (int i = 0; i < len; i++) {
    int iBar = map(iBuf[i], -2048, 2047, -20, 20);
    int qBar = map(qBuf[i], -2048, 2047, -20, 20);
    Serial.printf("  [%3d] I=%5d ", i, iBuf[i]);
    for (int j = 0; j < 40; j++) {
      Serial.write(j == 20 + iBar ? 'I' : (j == 20 ? '|' : ' '));
    }
    Serial.printf("  Q=%5d\n", qBuf[i]);
  }
}

// ── Export IQ as raw binary ──────────────────────────────────
void exportIQ() {
  Serial.printf("IQ_DATA:%d:", IQ_BUFFER_SIZE);
  for (int i = 0; i < IQ_BUFFER_SIZE; i++) {
    Serial.write((uint8_t)(iBuffer[i] & 0xFF));
    Serial.write((uint8_t)((iBuffer[i] >> 8) & 0xFF));
    Serial.write((uint8_t)(qBuffer[i] & 0xFF));
    Serial.write((uint8_t)((qBuffer[i] >> 8) & 0xFF));
  }
  Serial.println("\nIQ export complete.");
}

// ── Export demodulated data ──────────────────────────────────
void exportDemod(const char* type, float* data, int len) {
  Serial.printf("DEMOD_%s:%d:", type, len);
  for (int i = 0; i < len; i++) {
    uint16_t val = (uint16_t)((data[i] + 1.0f) * 32767.5f);
    Serial.write((uint8_t)(val & 0xFF));
    Serial.write((uint8_t)((val >> 8) & 0xFF));
  }
  Serial.printf("\n%s export complete.\n", type);
}
