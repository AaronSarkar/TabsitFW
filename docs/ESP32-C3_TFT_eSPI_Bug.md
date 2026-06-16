# ESP32-C3 + TFT_eSPI Setup Guide

This document covers the issues and fixes required to get TFT_eSPI working
on the XIAO ESP32-C3 with an ST7789 display (284×76 landscape).

---

## 1. ESP32 Arduino Core Crash (Reboot Loop)

### Symptom
Calling `tft.init()` causes an infinite reboot loop with
`Guru Meditation Error: Core panic'ed (StoreProhibited)` at `EXCVADDR 0x00000010`.

### Cause
ESP32 Arduino package **≥ 2.0.15** (PlatformIO espressif32 ≥ 6.7.0) changed
SPI register definitions. TFT_eSPI's workarounds in
`TFT_eSPI_ESP32_C3.h:30-35` override the correct register addresses,
causing writes to invalid memory.

### Fix
Pin the PlatformIO platform to the last compatible version:

```ini
platform = espressif32@6.6.0     ; Arduino 2.0.14
```

---

## 2. White Screen (No Display Output)

### Cause A: MISO pin aliased to MOSI
On ESP32-C3, if `TFT_MISO = -1`, the library forces `TFT_MISO = TFT_MOSI`,
sharing both signals on one pin. This locks up the SPI peripheral in newer
cores.

### Fix
Set MISO to a valid GPIO pin (even if unused):

```ini
-D TFT_MISO=9                   ; D9 on XIAO, must be valid GPIO
```

### Cause B: Wrong DC pin GPIO number
The XIAO ESP32-C3 pin numbering does not match the silkscreen labels:

| Label | GPIO |
|-------|------|
| D3    | 5    |
| **D4** | **6** |
| D6    | 21   |
| D7    | 20   |
| D8    | 8    |
| D9    | 9    |
| D10   | 10   |

If your wiring uses D4, set `TFT_DC=6` (not 4).

### Fix
```ini
-D TFT_DC=6                     ; D4 on XIAO = GPIO6
```

---

## 3. Display Rotation & GRAM Offsets

An ST7789 with 284×76 landscape has a 76×284 native (portrait) GRAM.
To center the visible area in a 240×320 GRAM, offsets are required.

### Fix: Add resolution case to `ST7789_Rotation.h`

Add `_init_width == 76` branches in all four rotation cases with these offsets
(matched to the working custom driver):

```
Rotation 0 (portrait):       colstart=82, rowstart=18
Rotation 1 (landscape 90°):  colstart=18, rowstart=82
Rotation 2 (inverted port):  colstart=0,  rowstart=80
Rotation 3 (inverted land):  colstart=0,  rowstart=0
```

The file is at:
```
.pio/libdeps/seeed_xiao_esp32c3/TFT_eSPI/TFT_Drivers/ST7789_Rotation.h
```

Also define `CGRAM_OFFSET` in build flags:
```ini
-D CGRAM_OFFSET=1
```

---

## 4. Display Initialisation Sequence

The default TFT_eSPI init for ST7789 sends `fillScreen(TFT_RED)` over a
240×320 window (76 800 pixels), which can cause brownouts on the XIAO.

### Fix: Replace init sequence in `ST7789_Init.h`

Replace the first init branch with the working minimal sequence from the
custom driver (no `fillScreen`, correct register values):

```
.pio/libdeps/seeed_xiao_esp32c3/TFT_eSPI/TFT_Drivers/ST7789_Init.h
```

Commands used (matching the custom driver):
```
SWRESET  → delay 150
SLPOUT   → delay 150
COLMOD   = 0x55 (16-bit)
PORCTRL  = 0x0C 0x0C 0x00 0x33 0x33
GCTRL    = 0x35
VCOMS    = 0x19
LCMCTRL  = 0x2C
VDVVRHEN = 0x01
VRHS     = 0x12
VDVSET   = 0x20
FRCTR2   = 0x0F
PWCTRL1  = 0xA4 0xA1
PVGAMCTRL gamma table
NVGAMCTRL gamma table
INVOFF
NORON    → delay 10
DISPON   → delay 150
```

---

## 5. Complete `platformio.ini`

```ini
[env:seeed_xiao_esp32c3]
platform = espressif32@6.6.0
board = seeed_xiao_esp32c3
framework = arduino

lib_deps =
    bodmer/TFT_eSPI @ ^2.5.43

build_flags =
    -D USER_SETUP_LOADED=1
    -D ST7789_DRIVER=1
    -D CGRAM_OFFSET=1
    -D TFT_WIDTH=76
    -D TFT_HEIGHT=284
    -D TFT_RGB_ORDER=TFT_BGR

    -D TFT_MISO=9
    -D TFT_MOSI=10
    -D TFT_SCLK=8
    -D TFT_CS=20
    -D TFT_DC=6
    -D TFT_RST=5

    -D LOAD_FONT2=1
    -D LOAD_FONT4=1
    -D SPI_FREQUENCY=4000000
```

## 6. Minimal `main.cpp`

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define PIN_BL 21

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);

    pinMode(PIN_BL, OUTPUT);
    digitalWrite(PIN_BL, LOW);   // Active LOW backlight

    tft.init();
    tft.setRotation(1);          // Landscape 284 x 76

    tft.fillScreen(TFT_NAVY);
    tft.drawRect(0, 0, tft.width(), tft.height(), TFT_YELLOW);
    tft.drawString("Hello!", 10, tft.height() / 2, 2);
}

void loop() {}
```

---

## 7. References

- https://github.com/Bodmer/TFT_eSPI/issues/3284
- https://github.com/Bodmer/TFT_eSPI/issues/3743
- https://github.com/espressif/arduino-esp32/issues/9618
