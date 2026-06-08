
#include <Arduino.h>
#include <Wire.h>
#include "BluetoothA2DPSink.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_WIDTH  128 // here change it to your display resolution
#define OLED_HEIGHT  64 //
#define OLED_ADDR   0x3C   

#define I2S_DIN    25
#define I2S_BCLK   26
#define I2S_LRC    27


BluetoothA2DPSink  a2dp_sink;
Adafruit_SSD1306   display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

struct TrackInfo {
  char   title[128];
  char   artist[128];
  char   album[128];
  bool   playing;
};

static TrackInfo pendingTrack  = { "Waiting...", "Connect via BT", "", false };
static TrackInfo activeTrack   = { "Waiting...", "Connect via BT", "", false };

volatile bool    trackChanged      = false;
volatile bool    playStateChanged  = false;

struct ScrollState {
  int  offset      = 0;
  int  textWidth   = 0;
  bool needsScroll = false;
};

ScrollState titleScroll;
ScrollState artistScroll;

uint8_t  barHeights[8]     = {4, 6, 12, 8, 10, 5, 14, 7};
uint8_t  barTargets[8]     = {4, 6, 12, 8, 10, 5, 14, 7};
uint32_t lastBarUpdate     = 0;
uint32_t lastScrollUpdate  = 0;
uint32_t lastDisplayUpdate = 0;
uint32_t lastPulseUpdate   = 0;
uint8_t  pulsePhase        = 0;
uint8_t  dotPhase          = 0;


volatile int16_t peakLevel = 0;


static const uint8_t PROGMEM spotify_bmp[] = {
  0x03, 0xC0, 0x0F, 0xF0, 0x1F, 0xF8, 0x3F, 0xFC,
  0x7F, 0xFE, 0x7E, 0xFE, 0xFF, 0xFC, 0xFF, 0x80,
  0xFF, 0xE0, 0xFF, 0xFC, 0x7F, 0xF8, 0x7C, 0x70,
  0x3F, 0xFC, 0x1F, 0xF8, 0x0F, 0xF0, 0x03, 0xC0
};
void avrc_metadata_callback(uint8_t attr_id, const uint8_t *text) {
  const char *val = (const char *)text;
  switch (attr_id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      strlcpy(pendingTrack.title,  (val && val[0]) ? val : "Unknown Title",  sizeof(pendingTrack.title));
      trackChanged = true;
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      strlcpy(pendingTrack.artist, (val && val[0]) ? val : "Unknown Artist", sizeof(pendingTrack.artist));
      trackChanged = true;
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      strlcpy(pendingTrack.album,  (val && val[0]) ? val : "",               sizeof(pendingTrack.album));
      trackChanged = true;
      break;
  }
}
void connection_state_callback(esp_a2d_connection_state_t state, void *) {
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    pendingTrack.playing = false;
    trackChanged = true;
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    strlcpy(pendingTrack.title,  "Waiting...",     sizeof(pendingTrack.title));
    strlcpy(pendingTrack.artist, "Connect via BT", sizeof(pendingTrack.artist));
    strlcpy(pendingTrack.album,  "",               sizeof(pendingTrack.album));
    pendingTrack.playing = false;
    trackChanged = true;
  }
}

void audio_state_callback(esp_a2d_audio_state_t state, void *) {
  pendingTrack.playing = (state == ESP_A2D_AUDIO_STATE_STARTED);
  playStateChanged = true;
}



int strPixelWidth(const char *s) {
  int16_t  x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)w;
}

void resetScrollState() {
  display.setTextSize(1);

  titleScroll.offset      = 0;
  titleScroll.textWidth   = strPixelWidth(activeTrack.title);
  titleScroll.needsScroll = (titleScroll.textWidth > OLED_WIDTH);

  artistScroll.offset      = 0;
  artistScroll.textWidth   = strPixelWidth(activeTrack.artist);
  artistScroll.needsScroll = (artistScroll.textWidth > OLED_WIDTH);
}

void drawScrollText(const char *text, int y, ScrollState &ss) {
  display.setCursor(-ss.offset, y);
  display.print(text);
  
  if (ss.needsScroll) {
    display.setCursor(-ss.offset + ss.textWidth + 20, y);
    display.print(text);
  }
}

void updateScroll() {
  if (titleScroll.needsScroll) {
    titleScroll.offset++;
    if (titleScroll.offset >= titleScroll.textWidth + 20)
      titleScroll.offset = 0;
  }
  if (artistScroll.needsScroll) {
    artistScroll.offset++;
    if (artistScroll.offset >= artistScroll.textWidth + 20)
      artistScroll.offset = 0;
  }
}


void updateBars() {
  for (int i = 0; i < 8; i++) {
    if      (barHeights[i] < barTargets[i]) barHeights[i]++;
    else if (barHeights[i] > barTargets[i]) barHeights[i]--;
    else                                    barTargets[i] = (uint8_t)random(3, 20);
  }
}

void drawEqualizer(int x, int baseY, int barW, int gap) {
  for (int i = 0; i < 8; i++) {
    int bx = x + i * (barW + gap);
    int bh = barHeights[i];
    display.fillRect(bx, baseY - bh, barW, bh, WHITE);
  }
}

void drawSpotifyPulse(int cx, int cy) {
  uint8_t r1 = 12 + (pulsePhase % 4);
  display.drawCircle(cx, cy, r1, WHITE);
  display.drawCircle(cx, cy, 10, WHITE);
  display.fillCircle(cx, cy,  7, WHITE);

  display.drawFastHLine(cx - 4, cy + 2,  8, BLACK);
  display.drawFastHLine(cx - 5, cy,      10, BLACK);
  display.drawFastHLine(cx - 3, cy - 2,  6, BLACK);
}


void drawVolumeBar(int pct) {
  const int bx = 0, by = 56, bw = 128, bh = 6;
  display.drawRect(bx, by, bw, bh, WHITE);
  int fill = (int)((pct / 100.0f) * (bw - 2));
  if (fill > 0)
    display.fillRect(bx + 1, by + 1, fill, bh - 2, WHITE);
}


void drawConnectingScreen() {
  display.clearDisplay();
  display.drawBitmap(56, 6, spotify_bmp, 16, 16, WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(24, 28);
  display.print(F("ESP32-BTSPK"));

  display.setCursor(28, 42);
  display.print(F("Connecting"));
  for (int i = 0; i < (int)(dotPhase % 4); i++) display.print('.');

  display.display();
}


void drawPlaybackScreen() {
  display.clearDisplay();


  display.drawBitmap(0, 0, spotify_bmp, 16, 16, WHITE);

  if (activeTrack.playing) {
    // ▶ play triangle
    display.fillTriangle(110, 2, 110, 12, 120, 7, WHITE);
  } else {
    // ▌▌ pause bars
    display.fillRect(110, 2, 3, 11, WHITE);
    display.fillRect(116, 2, 3, 11, WHITE);
  }

  display.drawFastHLine(0, 17, 128, WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  drawScrollText(activeTrack.title,  20, titleScroll);

  drawScrollText(activeTrack.artist, 31, artistScroll);

  if (activeTrack.album[0] != '\0') {
    char albBuf[22];
    strlcpy(albBuf, activeTrack.album, sizeof(albBuf));
    if (strlen(activeTrack.album) > 21) {
      albBuf[20] = '~';
      albBuf[21] = '\0';
    }
    display.setCursor(0, 42);
    display.print(albBuf);
  }

  if (activeTrack.playing) {
    drawEqualizer(64, 53, 3, 2);
  } else {
    drawSpotifyPulse(112, 43);
  }

  int lvl = min(100, (int)((long)abs((int)peakLevel) * 100 / 32768));
  drawVolumeBar(lvl);

  display.display();
}

void i2s_data_callback(const uint8_t *data, uint32_t len) {
  const int16_t *samples = (const int16_t *)data;
  uint32_t count = len / 2;
  int16_t  maxV  = 0;
  for (uint32_t i = 0; i < count; i++) {
    int16_t s = abs(samples[i]);
    if (s > maxV) maxV = s;
  }
  peakLevel = maxV;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n╔══════════════════════════════╗"));
  Serial.println(F("║  ESP32 BT Speaker + OLED     ║"));
  Serial.println(F("╚══════════════════════════════╝"));

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[OLED] Init FAILED – check wiring!"));
    for (;;) delay(1000);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextWrap(false);   // essential: prevents GFX from wrapping scrolled text

  // Splash screen
  display.drawBitmap(56, 10, spotify_bmp, 16, 16, WHITE);
  display.setTextSize(1);
  display.setCursor(24, 34);
  display.print(F("ESP32-BTSPK"));
  display.setCursor(20, 48);
  display.print(F("Initialising..."));
  display.display();
  delay(1500);

  i2s_pin_config_t i2s_pins = {
    .bck_io_num   = I2S_BCLK,
    .ws_io_num    = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };

  a2dp_sink.set_pin_config(i2s_pins);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_on_connection_state_changed(connection_state_callback);
  a2dp_sink.set_on_audio_state_changed(audio_state_callback);

  a2dp_sink.start("ESP32-BTSPK");

  Serial.println(F("[BT]   A2DP sink → 'ESP32-BTSPK'"));
  Serial.println(F("[OLED] Ready"));

  resetScrollState();
  randomSeed(esp_random());
}

void loop() {
  uint32_t now = millis();

  if (trackChanged || playStateChanged) {
  
    memcpy(&activeTrack, &pendingTrack, sizeof(TrackInfo));

    if (trackChanged) {
      trackChanged = false;
      resetScrollState();
      Serial.printf("[TRACK] %s — %s (%s)\n",
                    activeTrack.title, activeTrack.artist, activeTrack.album);
    }
    if (playStateChanged) {
      playStateChanged = false;
      Serial.printf("[STATE] %s\n", activeTrack.playing ? "Playing" : "Paused");
    }
  }


  if (now - lastScrollUpdate >= 80) {
    lastScrollUpdate = now;
    updateScroll();
  }


  if (now - lastBarUpdate >= 100) {
    lastBarUpdate = now;
    if (activeTrack.playing) updateBars();
  }


  if (now - lastPulseUpdate >= 400) {
    lastPulseUpdate = now;
    pulsePhase++;
    dotPhase++;
  }

  if (now - lastDisplayUpdate >= 50) {
    lastDisplayUpdate = now;

    bool connected = (a2dp_sink.get_connection_state() ==
                      ESP_A2D_CONNECTION_STATE_CONNECTED);

    if (!connected) {
      drawConnectingScreen();
    } else {
      drawPlaybackScreen();
    }
  }
}
