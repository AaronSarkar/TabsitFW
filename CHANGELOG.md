# Session Summary — ST7789 Display on XIAO ESP32-C3

## Files Changed / Created

| File | Status | Purpose |
|---|---|---|
| `platformio.ini` | Modified | Removed TFT_eSPI, added Adafruit GFX |
| `include/tft_setup.h` | Orphaned | TFT_eSPI config (no longer used, kept for reference) |
| `src/display/display.h` | **New** | Custom ST7789 driver class (inherits Adafruit_GFX) |
| `src/display/display.cpp` | **New** | Driver implementation (SPI, init, pixel/rect drawing) |
| `src/ui/ui.h` | Cleared | Was LVGL UI (deferred) |
| `src/ui/ui.cpp` | Cleared | Was LVGL UI (deferred) |
| `src/main.cpp` | Rewritten | Simple test: fill + text |

Unchanged: `src/models/task.h/cpp`, `src/input/encoder.h/cpp`, `src/config/pins.h`

---

## Critical Discoveries

### 1. XIAO ESP32-C3 Pin Mapping (D-pin != GPIO)

```
D0=GPIO2   D1=GPIO3   D2=GPIO4   D3=GPIO5
D4=GPIO6   D5=GPIO7   D6=GPIO21  D7=GPIO20
D8=GPIO8   D9=GPIO9   D10=GPIO10
```

So `D7` is **GPIO20**, not GPIO7. This caused all early pin definitions to be wrong.

### 2. Backlight Polarity — Active LOW

Backlight turns ON with `digitalWrite(D6, LOW)` (GPIO21). Set `TFT_BACKLIGHT_ON=0`.

### 3. CGRAM Offsets (Portrait)

Calibrated by sweeping row/col positions until the image filled the screen edge-to-edge:

| Orientation | colstart | rowstart |
|---|---|---|
| Portrait (rot=0) | 82 | 18 |
| Landscape (rot=1) | 20 | 0 (rough — not yet calibrated) |

These are set in `display.cpp` via `_colOff` / `_rowOff`.

### 4. TFT_eSPI Incompatibility

TFT_eSPI v2.5.43 crashes on the XIAO ESP32-C3 because its `CS_L`/`DC_C` macros use **direct GPIO register access** (`GPIO.out_w1tc.val`). This works on Xtensa ESP32 but fails on the RISC-V ESP32-C3, causing a watchdog reset.

The same init sequence works fine when done manually with `digitalWrite()` and `SPI.transfer()`.

---

## Current Display Driver (`src/display/`)

The `Display` class inherits `Adafruit_GFX` and implements the display backend with:
- `digitalWrite()` for CS/DC control (no direct register access)
- `SPI.transfer()` for pixel data
- ST7789 init sequence (same as the manual test that worked)
- Rotation 0 supports calibrated CGRAM offsets (col=82, row=18)
- Rotation 1-3 use guessed offsets (need calibration)

Available API:
- `begin()` — init pins, SPI, and display
- `setRotation(r)` — 0-3
- `fillScreen(color)`, `fillRect(x,y,w,h,color)`
- `drawPixel(x,y,color)`
- All `Adafruit_GFX` methods: `setCursor`, `print`, `println`, `drawLine`, `drawCircle`, etc.

---

## Next Steps

### Known Issue — Rotation not working
The `setRotation()` command sends MADCTL via `cmd()` which needs an active SPI transaction. Currently `setRotation` doesn't call `SPI.beginTransaction()` before sending. Fix: wrap the MADCTL write in `SPI.beginTransaction()` / `SPI.endTransaction()`.

### Short-term
1. **Calibrate rotation 1 (landscape) offsets** — the current `_colOff=20, _rowOff=0` need adjustment. Do an offset sweep similar to what we did for portrait.
2. **Verify rotation 2 and 3** — offsets may need calibration for those too.

### Medium-term
3. **Add LVGL** — once the display driver is stable, integrate LVGL with a custom flush callback using our `Display` class.
4. **Build task list UI** — use LVGL to implement the scrollable task list with encoder navigation.
5. **Revisit TFT_eSPI** — if LVGL or performance requires it, investigate overriding TFT_eSPI's CS/DC macros to use `digitalWrite()` instead of direct register access. The `tft_setup.h` file has the correct pin configs ready.

### Long-term
6. **Bluetooth task sync** — populate tasks from a phone app via BLE.
7. **Future widgets** — the UI architecture (LVGL) should support adding weather, calendar, etc. as new "screens."

---

## Reference: Pin Table

| Function | XIAO Pin | GPIO | Current Define |
|---|---|---|---|
| SW (Button) | D0 | 2 | `BUTTON` in pins.h |
| DT (Encoder B) | D1 | 3 | `ENCODER_B` in pins.h |
| CLK (Encoder A) | D2 | 4 | `ENCODER_A` in pins.h |
| RES | D3 | 5 | `PIN_RST` in display.h |
| DC | D4 | 6 | `PIN_DC` in display.h |
| SDA (MOSI) | D10 | 10 | `PIN_MOSI` in display.h |
| SCL (SCK) | D8 | 8 | `PIN_SCLK` in display.h |
| CS | D7 | 20 | `PIN_CS` in display.h |
| BLK | D6 | 21 | `PIN_BL` in display.h |
| (unused) | D9 | 9 | `PIN_MISO` in display.h |

Note: encoder pins use symbolic `D0`/`D1`/`D2` which resolve via `pins_arduino.h` to the correct GPIO numbers. Display pins are hardcoded as GPIO numbers in `display.h`.
