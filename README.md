# ESP32-BTSPK — Bluetooth Speaker with Spotify-Style OLED Display

A Bluetooth A2DP audio sink built on the ESP32 that streams audio to a **MAX98357A I2S amplifier** and displays live track metadata (song, artist, album) on a **128×64 SSD1306 OLED** with a Spotify-inspired animated UI.

---

## Features

- 🎵 **Bluetooth A2DP sink** — connect any phone, tablet, or computer and stream audio wirelessly
- 📋 **AVRCP metadata** — receives and displays song title, artist, and album in real time
- 📺 **Animated OLED UI** — Spotify-style display with:
  - Scrolling title and artist text for long names
  - Animated equaliser bars while playing
  - Spotify pulse animation while paused
  - Play / Pause indicator icon
  - Live VU meter bar (optional, see below)
  - "Connecting…" splash screen with animated dots
- 🔊 **I2S audio output** via MAX98357A — clean digital-to-analog conversion

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32 Dev Module |
| Amplifier | MAX98357A (I2S, mono or stereo) |
| Display | SSD1306 OLED 128×64 (I2C) |

### Pin Map

| Signal | GPIO |
|--------|------|
| OLED SDA | 21 |
| OLED SCL | 22 |
| I2S DIN (data) | 25 |
| I2S BCLK (bit clock) | 26 |
| I2S LRC (word select) | 27 |

> **OLED I2C address:** `0x3C` (default). Change `OLED_ADDR` in the sketch to `0x3D` if your module uses that address.

---

## Schematic (quick reference)

```
ESP32          MAX98357A
GPIO 25  ───►  DIN
GPIO 26  ───►  BCLK
GPIO 27  ───►  LRC
3.3 V    ───►  VDD
GND      ───►  GND

ESP32          SSD1306 OLED
GPIO 21  ───►  SDA
GPIO 22  ───►  SCL
3.3 V    ───►  VCC
GND      ───►  GND
```

---

## Getting Started

See [INSTALLATION.md](INSTALLATION.md) for full setup instructions and [REQUIREMENTS.md](REQUIREMENTS.md) for all dependencies.

---

## Usage

1. Flash the sketch to your ESP32.
2. The OLED shows **"ESP32-BTSPK — Connecting…"**
3. On your phone/computer, open Bluetooth settings and pair with **`ESP32-BTSPK`**.
4. Play any audio — the display switches to the playback screen automatically.
5. Track metadata updates whenever you skip a track (requires AVRCP support on the source device).

### Optional: Live VU Meter

The VU bar at the bottom of the display can react to real audio levels instead of showing a static zero. Uncomment this line in `setup()`:

```cpp
// a2dp_sink.set_stream_reader(i2s_data_callback, false);
```

> Note: this uses extra CPU cycles; disable if you experience audio glitches.

---

## Project Structure

```
BTSPK.ino   — main sketch (all logic in a single file)
README.md
INSTALLATION.md
REQUIREMENTS.md
```

---

## License

MIT License — free to use, modify, and distribute. Attribution appreciated.
