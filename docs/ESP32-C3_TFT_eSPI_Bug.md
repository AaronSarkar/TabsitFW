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

> **IMPORTANT — do NOT patch the library for this.**
> A previous version of this guide patched `_init_width == 76` branches into
> `.pio/libdeps/.../TFT_eSPI/TFT_Drivers/ST7789_Rotation.h`. Files under
> `.pio/libdeps/` are **wiped whenever PlatformIO re-downloads the library**
> (clean build, fresh checkout, version bump), which silently breaks the
> display (image shifted, looks "glitched"). The fix now lives in our own
> tracked source so it survives library reinstalls.

### Fix: Apply the offset in our own `disp_flush()`

We use rotation 1 (landscape). The required offsets for this panel are
`colstart=18, rowstart=82`. Adding these to the address window in our flush
callback is mathematically identical to having `colstart`/`rowstart` set
inside the library:

```cpp
// src/display/display.cpp
constexpr uint16_t DISP_COLSTART = 18;
constexpr uint16_t DISP_ROWSTART = 82;
...
tft_instance->setAddrWindow(area->x1 + DISP_COLSTART,
                            area->y1 + DISP_ROWSTART, w, h);
```

For reference, the offsets for the other rotations are:
```
Rotation 0 (portrait):       colstart=82, rowstart=18
Rotation 2 (inverted port):  colstart=0,  rowstart=80
Rotation 3 (inverted land):  colstart=0,  rowstart=0
```

---

## 4. Display Colour Inversion

The default TFT_eSPI init for ST7789 sends `fillScreen(TFT_RED)` over a
240×320 window (76 800 pixels), which can cause brownouts on the XIAO.

> **IMPORTANT — do NOT patch `ST7789_Init.h`.** As with the offsets above, any
> edits to `.pio/libdeps/.../TFT_eSPI/TFT_Drivers/ST7789_Init.h` are wiped on
> library reinstall.

### Fix: Force inversion off from our own code

After `tft.begin()` / `tft.setRotation(1)`, override the library default:

```cpp
// src/main.cpp
tft.begin();
tft.setRotation(1);
tft.invertDisplay(false);   // stock init leaves INVON on
```

If colours ever look inverted (negative-looking), flip this to
`tft.invertDisplay(true)`.

<!-- Historical reference: the calibrated init sequence from the old custom
driver (no longer applied via library patch; kept for documentation only).
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
INVOFF  (this is what tft.invertDisplay(false) reproduces at runtime)
NORON    → delay 10
DISPON   → delay 150
```
-->

---

## 5. Project configuration

The TFT_eSPI configuration is **not** kept as inline `-D` build flags. Instead
it lives in `include/tft_setup.h`, which is force-included into every
translation unit (including the TFT_eSPI library build) via the `-include`
flag. This keeps the pin map / driver settings in one readable place.

### `platformio.ini`

```ini
[env:seeed_xiao_esp32c3]
platform = espressif32@6.6.0     ; pinned: Arduino 2.0.14 (see Section 1)
board = seeed_xiao_esp32c3
framework = arduino
upload_port = COM7
monitor_port = COM7
upload_speed = 115200
monitor_speed = 115200
monitor_filters = esp32_exception_decoder, colorize
lib_deps =
    bodmer/TFT_eSPI @ ^2.5
    lvgl/lvgl@^8.3
build_flags =
    -include include/tft_setup.h   ; force-include the TFT_eSPI config header
    -DLV_CONF_INCLUDE_SIMPLE
    -I include                     ; so the LVGL lib build can find lv_conf.h
    -DDIAG_LEVEL=2
build_src_filter = +<*> -<display/lvgl_display.cpp>
```

### `include/tft_setup.h` (the actual TFT_eSPI config)

```cpp
#define ST7789_DRIVER
#define TFT_CS      20     // D7
#define TFT_DC      6      // D4
#define TFT_RST     5      // D3
#define TFT_MOSI    10     // D10
#define TFT_SCLK    8      // D8
#define TFT_BL      21     // D6
#define TFT_MISO    9      // D9 (unused, satisfies SPI)
#define TFT_BACKLIGHT_ON 0 // Active LOW

#define TFT_WIDTH    76
#define TFT_HEIGHT   284
#define CGRAM_OFFSET       // declared, but offsets are applied in disp_flush (Section 3)

#define SPI_FREQUENCY   40000000  // drop to 27000000 if you see signal noise
#define SUPPORT_TRANSACTIONS

#define LOAD_FONT2
#define LOAD_FONT4
#define USER_SETUP_LOADED
```

> Note: the `COLSTART`/`ROWSTART` macros that used to live here are no longer
> used by the library (it was reset to stock). The real offsets are applied in
> `src/display/display.cpp` — see Section 3.

## 6. Minimal `main.cpp`

This is a bare sanity-check sketch that draws **directly** with TFT_eSPI (it
does not go through LVGL's `disp_flush`), so the CGRAM offset from Section 3 is
not applied here — expect the test graphics to be shifted on the panel. The
real app applies the offset in `disp_flush`.

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
    tft.invertDisplay(false);    // stock init leaves INVON on (see Section 4)

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
