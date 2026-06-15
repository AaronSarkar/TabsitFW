# CHANGELOG — ST7789 Display on XIAO ESP32-C3

All notable changes to this project.

---

## v0_display — TFT_eSPI Migration

### Overview

Transitioned from a custom `Adafruit_GFX`-based display driver to the TFT_eSPI library on the XIAO ESP32-C3 with an ST7789 display (284×76 landscape).

---

### Files Changed

| File | Change |
|---|---|
| `platformio.ini` | Replaced `Adafruit GFX Library` with `bodmer/TFT_eSPI @ ^2.5`; pinned platform to `espressif32@6.6.0` (Arduino 2.0.14); added `-include include/tft_setup.h` and `build_src_filter = +<*> -<display/>` to exclude dead custom driver |
| `include/tft_setup.h` | **New** — TFT_eSPI `User_Setup_Select.h` config with ST7789 driver, GPIO pin mapping, CGRAM offsets (COLSTART=82, ROWSTART=18), SPI_FREQUENCY=4000000, CGRAM_OFFSET, LOAD_FONT2/4 |
| `src/main.cpp` | Uses `TFT_eSPI tft` instead of custom `Display`; backlight control via `pinMode(BL, OUTPUT); digitalWrite(BL, LOW)`; text drawn with `drawString()` (font 2) |
| `src/config/pins.h` | Removed `TFT_*` D-pin aliases (conflicted with `tft_setup.h` GPIO numbers); kept encoder pins only |
| `.pio/libdeps/.../ST7789_Init.h` | **Patched** — Replaced default init (brownout-causing `fillScreen(TFT_RED)` plus `INVON`) with working minimal sequence from custom driver: SWRESET → SLPOUT → COLMOD → PORCTRL → GCTRL → VCOMS → LCMCTRL → VDVVRHEN → VRHS → VDVSET → FRCTR2 → PWCTRL1 → gamma tables → INVOFF → NORON → DISPON |
| `.pio/libdeps/.../ST7789_Rotation.h` | **Patched** — Added `_init_width == 76` CGRAM offset branches for all four rotations (rot0: 82,18; rot1: 18,82; rot2: 0,80; rot3: 0,0); replaced `TFT_MAD_COLOR_ORDER` with `TFT_MAD_RGB` to force RGB color order |
| `src/display/display.h` | Excluded from build (dead code, depends on removed Adafruit GFX) |

---

### Issues Resolved

| # | Symptom | Root Cause | Fix |
|---|---|---|---|
| 1 | Reboot loop (Guru Meditation StoreProhibited) | Arduino ≥2.0.15 changed SPI register defs; TFT_eSPI's workarounds wrote to invalid memory | `platform = espressif32@6.6.0` (Arduino 2.0.14) |
| 2 | White screen / no display | `TFT_MISO=-1` aliased MISO→MOSI locking SPI; `TFT_DC=4` was wrong GPIO | `TFT_MISO=9`; `TFT_DC=6` |
| 3 | Brownout during init | Default init sent `fillScreen(TFT_RED)` over 240×320 (76800 px) | Replaced with minimal init (no fillScreen) |
| 4 | Display off-center | CGRAM offsets not configured for 76×284 on 240×320 panel | Added `_init_width == 76` cases to ST7789_Rotation.h with calibrated offsets |
| 5 | Colors swapped (yellow↔cyan, red↔blue) | `TFT_RGB_ORDER` logic in ST7789_Defines.h checks `#if (TFT_RGB_ORDER == 1)` but `TFT_RGB` evaluates to 0; also `CGRAM_OFFSET` defaulted to BGR | Replaced `TFT_MAD_COLOR_ORDER` with `TFT_MAD_RGB` directly in rotation file |
| 6 | Text not rendering with `print()`/`println()` | Font 1 (GLCD) `write()` path broken in TFT_eSPI v2.5.43 on ESP32-C3 | Used `drawString()` with font 2 instead |
| 7 | Backlight off | TFT_eSPI stuck no default pulse to BL pin | Manual `pinMode(BL, OUTPUT); digitalWrite(BL, LOW)` in `setup()` |

---

### Current State

- **Screen**: 284×76 landscape, navy background, white text, yellow border, colored corner markers
- **Displayed text**: "Hello XIAO!", "284x76 landscape", "TFT_eSPI"
- **Rotation**: 1 (landscape) — calibrated CGRAM offsets
- **Font**: font 2 via `drawString()`
- **Colors**: RGB mode (no BGR swap)

### Remaining

- `print()`/`println()` with font 1 still broken — use `drawString()` as workaround
- Rotations 2 and 3 have guessed CGRAM offsets (not calibrated)
- SPI frequency at 4 MHz (matching working custom driver)
