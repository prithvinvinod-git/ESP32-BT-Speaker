# Installation Guide — ESP32-BTSPK

## 1. Install Arduino IDE

Download and install **Arduino IDE 2.x** from [arduino.cc/en/software](https://www.arduino.cc/en/software).

---

## 2. Add ESP32 Board Support

1. Open Arduino IDE → **File → Preferences**
2. Paste the following URL into *Additional boards manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for `esp32` and install **esp32 by Espressif Systems** (version **2.x** or later)

---

## 3. Install Required Libraries

Open **Tools → Manage Libraries** and install each of the following:

| Library | Author | Minimum Version |
|---------|--------|----------------|
| ESP32-A2DP | Phil Schatzmann | 1.7.0 |
| Adafruit SSD1306 | Adafruit | 2.5.0 |
| Adafruit GFX Library | Adafruit | 1.11.0 |

Search by name in the Library Manager and click **Install**. Accept any dependency prompts.

---

## 4. Configure Board Settings

Go to **Tools** and set:

| Setting | Value |
|---------|-------|
| Board | ESP32 Dev Module |
| CPU Frequency | 240 MHz |
| Flash Size | 4MB |
| Partition Scheme | Default 4MB with spiffs |
| Upload Speed | 921600 |
| Port | (your ESP32 COM/serial port) |

---

## 5. Open and Upload the Sketch

1. Clone or download this repository.
2. Open `BTSPK.ino` in Arduino IDE.
3. Connect your ESP32 via USB.
4. Click **Upload** (→ arrow button).
5. Hold the **BOOT** button on the ESP32 if upload does not start automatically.

---

## 6. Verify

Open **Tools → Serial Monitor** at **115200 baud**. You should see:

```
╔══════════════════════════════╗
║  ESP32 BT Speaker + OLED     ║
╚══════════════════════════════╝
[BT]   A2DP sink → 'ESP32-BTSPK'
[OLED] Ready
```

The OLED will display the connecting splash screen. Pair your phone with **`ESP32-BTSPK`** and start playing audio.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `[OLED] Init FAILED` in Serial Monitor | Check SDA/SCL wiring; try changing `OLED_ADDR` to `0x3D` |
| Upload fails / times out | Hold BOOT button during upload |
| No audio output | Verify MAX98357A DIN/BCLK/LRC wiring matches pin map |
| Track metadata not updating | Source device may not support AVRCP; try a different player app |
| Audio crackling | Disable VU meter callback (`set_stream_reader`), or reduce CPU load |
