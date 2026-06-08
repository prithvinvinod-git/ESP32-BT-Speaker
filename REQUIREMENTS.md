# Requirements — ESP32-BTSPK

## Hardware Requirements

| Component | Specification |
|-----------|--------------|
| Microcontroller | ESP32 (any variant with Bluetooth Classic + I2S) |
| Audio Amplifier | MAX98357A breakout module (I2S input) |
| Display | SSD1306 OLED, 128×64 px, I2C interface |
| Power Supply | 5 V via USB or external regulated supply |
| Wires / Jumpers | Male-to-male or Dupont jumpers for breadboard connections |

> The sketch is developed for and tested on the standard **ESP32 Dev Module** (38-pin). Other ESP32 boards (e.g. ESP32-S3, ESP32-C3) are **not supported** — they have different I2S peripheral addresses and lack Bluetooth Classic required for A2DP.

---

## Software Requirements

### Arduino IDE
- **Arduino IDE 2.x** — [Download](https://www.arduino.cc/en/software)

### Board Package
- **esp32 by Espressif Systems** ≥ 2.0.0  
  (installed via Boards Manager — see [INSTALLATION.md](INSTALLATION.md))

### Libraries

| Library | Author | Version | Purpose |
|---------|--------|---------|---------|
| [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) | Phil Schatzmann | ≥ 1.7.0 | Bluetooth A2DP sink + AVRCP metadata |
| [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) | Adafruit | ≥ 2.5.0 | SSD1306 OLED driver |
| [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) | Adafruit | ≥ 1.11.0 | Graphics primitives for the display |

All libraries are available in the **Arduino Library Manager** (Tools → Manage Libraries).

### Built-in / Auto-included (no separate install needed)

| Library | Notes |
|---------|-------|
| `Arduino.h` | Core Arduino framework |
| `Wire.h` | I2C communication (bundled with ESP32 board package) |
| ESP-IDF Bluetooth stack | Bundled with the Espressif ESP32 board package |

---

## Bluetooth Requirements

- The **source device** (phone, computer, etc.) must support **Bluetooth Classic A2DP** profile for audio streaming.
- For track metadata (song title, artist, album), the source device must also support the **AVRCP** profile. Most modern Android and iOS devices do.
- Bluetooth Low Energy (BLE) only devices are **not compatible**.

---

## Resource Usage (approximate)

| Resource | Usage |
|----------|-------|
| Flash | ~700 KB (with Bluetooth stack) |
| RAM | ~100 KB at runtime |
| CPU | ~40% on one core (BT audio), ~5% on the other (display) |
| Partition | Default 4MB with spiffs |
