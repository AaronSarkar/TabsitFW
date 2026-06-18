# CHANGELOG — ST7789 Display on XIAO ESP32-C3

All notable changes to this project.

---

## v0_display — Frame-Rate & Serial Robustness (latest)

### Overview

Improved rendering frame rate (especially during scrolling/animation) and made the device responsive on a cold power-up even with no serial monitor attached.

---

### Issues Resolved

| # | Symptom | Root Cause | Fix |
|---|---|---|---|
| 1 | After depower/repower, scrolling became extremely delayed/broken (but was fine right after flashing/monitoring) | `Serial` is USB CDC; with verbose `DIAG_LEVEL=2` logging, on a cold boot with no host draining the port the CDC TX buffer fills and every `Serial.printf()` blocks, stalling the UI loop | `Serial.setTxTimeoutMs(0)` in `setup()` — serial writes drop instead of blocking when nothing is listening |
| 2 | Choppy scrolling / low frame rate | SPI clock was 4 MHz (inherited from the old custom driver); a full-screen flush (21,584 px x 16-bit) took ~86 ms (~11 FPS) just for the transfer | Raised `SPI_FREQUENCY` to 40 MHz (~9 ms/frame); raised LVGL refresh rate; trimmed per-frame overhead |

---

### Changes

- **SPI clock 4 MHz → 40 MHz** (`include/tft_setup.h`): the dominant frame-rate fix (~11 FPS → 100+ FPS transfer ceiling). Comment notes stepping down to 27/20 MHz if signal glitches appear.
- **LVGL refresh period 30 ms → 16 ms** (`include/lv_conf.h`): lifts the animation/scroll FPS cap from ~33 to ~60.
- **Removed per-flush serial logging** in `disp_flush` (`src/display/display.cpp`): ran every frame and throttled the achievable rate.
- **Main loop `delay(5) → delay(2)`** (`src/main.cpp`): services `lv_timer_handler()` finely enough to hit ~60 FPS.
- **Non-blocking serial** (`src/main.cpp`): `Serial.setTxTimeoutMs(0)`.

---

## v0_display — Gemini-Inspired UI Redesign + LVGL Tick Fix

### Overview

Resolved the long-standing Details-screen scrolling and screen-animation issues (single root cause: LVGL had no time source), then gave the UI a sleek, "liminal" Gemini-inspired redesign with a custom rounded font and a brand gradient accent.

---

### Issues Resolved

| # | Symptom | Root Cause | Fix |
|---|---|---|---|
| 1 | Details scrolling registered in software but never repainted; fade animations never advanced | LVGL was compiling with **default config** (`LV_TICK_CUSTOM = 0`) because the LVGL *library* build couldn't find `lv_conf.h` — the project include dir wasn't on the library's include path, so `#include "lv_conf.h"` silently fell back to defaults. With no tick source, the periodic refresh timer and animations never ran (only forced `lv_refr_now()` redraws worked). | Added `-I include` to `build_flags` so the LVGL library build resolves the project `lv_conf.h`; enabled `LV_TICK_CUSTOM` using `millis()`. Removed the duplicate `src/lv_conf.h` to make `include/lv_conf.h` the single source of truth. |
| 2 | Editing `lv_conf.h` had no effect | LVGL objects were cached and the broken dependency tracking never recompiled them. | Confirmed config now appears in LVGL's dependency files after a clean build. |
| 3 | Font generation build error (`lv_font_t has no member 'user_data'`) | Generated fonts set `.user_data` unconditionally. | Enabled `LV_USE_USER_DATA 1`. |

---

### Features / Changes

- **Title fade animation**: Re-enabled the home-screen title cross-fade on task change (was previously disabled due to the tick issue). Tuned to 150 ms in / 150 ms out for a snappy feel.
- **Custom rounded font**: Added **Quicksand** (OFL) converted to LVGL fonts — `font_quicksand_18` (SemiBold, titles) and `font_quicksand_13` (Medium, body). Tooling + regeneration steps in `tools/fonts/`.
- **Gemini-inspired dark theme**: Single consistent dark background (`#16171A`) across all screens — removed the grey header/footer bars entirely.
- **Priority dot**: Replaced the priority text label with a small colored dot (soft green/amber/red).
- **Gradient accent**: Thin blue→purple→pink gradient line along the bottom edge (3-stop gradient via `LV_GRADIENT_MAX_STOPS 3` / `LV_DRAW_COMPLEX 1`).
- **Details screen** restyled to match: consistent dark bg, rounded fonts, dropped the grey "Back" box, kept the now-working smooth scrolling.

---

### Files Changed

| File | Change |
|---|---|
| `platformio.ini` | Added `-I include` (so the LVGL library build finds `lv_conf.h`); added `-DDIAG_LEVEL=2` |
| `include/lv_conf.h` | Enabled `LV_TICK_CUSTOM` (millis), `LV_DRAW_COMPLEX`, `LV_GRADIENT_MAX_STOPS 3`, `LV_USE_USER_DATA` |
| `src/lv_conf.h` | **Removed** — duplicate config, replaced by single `include/lv_conf.h` |
| `src/ui/ui.cpp` | Redesigned Home + Details screens (dark theme, rounded fonts, priority dot, gradient accent); re-enabled + tuned title fade animation |
| `src/ui/fonts/font_quicksand_18.c`, `font_quicksand_13.c` | **New** — generated LVGL fonts |
| `tools/fonts/` | **New** — Quicksand TTFs + `lv_font_conv` setup + regeneration README |

---

## v0_display — LVGL UI with Rotary Encoder Input

### Overview

Implemented complete task display system with LVGL 8.x graphics library and rotary encoder input manager. The system displays tasks on a 284×76 ST7789 display with smooth scrolling navigation.

---

### Files Changed

| File | Change |
|---|---|
| `platformio.ini` | Added `lvgl @ ^8.4.0` library; added `-DLV_CONF_INCLUDE_SIMPLE` to build_flags |
| `include/lv_conf.h` | **New** — LVGL 8.x configuration with 284×76 resolution, RGB565 color format, 60FPS timer |
| `src/lv_conf.h` | **New** — LVGL configuration in src directory for proper include path |
| `src/display/lvgl_display.h` | **New** — LVGL display driver interface with TFT_eSPI integration |
| `src/display/lvgl_display.cpp` | **New** — LVGL display driver implementation with full-screen buffer (21584 pixels), flush callback, timer handler |
| `src/config/pins.h` | Fixed GPIO pin configuration (ENCODER_A=5, ENCODER_B=4, BUTTON=3, BL=21) - changed from Arduino D-pins to actual ESP32-C3 GPIO numbers |
| `src/input/encoder.h` | **New** — Input manager interface with event queue, InputEvent enum, InputLock states |
| `src/input/encoder.cpp` | **New** — Rotary encoder input manager with debounce, click detection (single/double/long-press), event queue system |
| `src/models/task.h` | Updated to match spec with String fields, Priority enum (PRIORITY_LOW/MEDIUM/HIGH), additional fields |
| `src/models/task.cpp` | Updated with example tasks matching spec, proper Priority enum values |
| `src/ui/ui.h` | **New** — UI interface with UIState enum, screen management functions |
| `src/ui/ui.cpp` | **New** — Complete UI implementation with Home Screen (header/content/footer), Details Screen, task scrolling, screen refresh, force refresh mechanism |
| `src/main.cpp` | Integrated LVGL display, input manager, and UI; added TFT direct test; removed Adafruit GFX dependency |

---

### Features Implemented

- **LVGL 8.x Graphics Library**: Full integration with TFT_eSPI driver for hardware-accelerated rendering
- **Rotary Encoder Input**: Complete input manager with event queue supporting rotation detection, single/double/long-click detection, event debouncing
- **Home Screen UI**: Task card layout with header (priority + position), content (task title), footer (due date)
- **Task Scrolling**: 2:1 encoder ratio for smooth navigation
- **Task Wrapping**: Seamless navigation from last to first task and vice versa
- **Screen Refresh**: Force refresh mechanism using lv_refr_now() and object invalidation
- **Serial Debugging**: Comprehensive debug output for input events and UI updates

---

### Cleanup & Details Screen Work (latest)

- **Removed scroll animation code**: Deleted the opacity/fade transition experiments (`scroll_anim_cb`, `startScrollAnimation`, `startEdgeResistanceAnimation`, `setTimeout` timer helpers) and related state. Home-screen scrolling now updates instantly, which is more reliable and less janky.
- **Removed boot TFT test**: Dropped the "TFT Test" splash/`fillScreen` test from `main.cpp` `setup()` so the device boots straight into the task UI.
- **Longer example descriptions**: `task.cpp` example tasks now have multi-sentence descriptions to exercise scrolling in the Details Screen.
- **Details Screen description scrolling (WIP)**: Reworked the Details content area to allow reading long descriptions. Rotation events in `UI_DETAILS` now route to `scrollDescription()` (1:1, no encoder ratio). Tried multiple approaches — manual `lv_obj_set_y`, LVGL native container scrolling (`lv_obj_scroll_by`), removing the flex layout, content-sized label, and disabling elastic/momentum scroll. **Still not working reliably** (see Known Issues).

---

### Issues Resolved

| # | Symptom | Root Cause | Fix |
|---|---|---|---|
| 1 | No display updates despite input detection | LVGL flush callback not being called | Added lv_refr_now() and object invalidation |
| 2 | Encoder counting twice per rotation | No ratio implementation | Added ENCODER_RATIO=2 in UI layer |
| 3 | Screen not changing on scroll | LVGL timer handler not called during init | Added forced LVGL refresh calls in setup() |
| 4 | Input detection not working | Wrong GPIO pin configuration (Arduino D-pins vs actual GPIO) | Changed to actual ESP32-C3 GPIO numbers |
| 5 | Guru Meditation crash with animation | LVGL animation system conflicting with screen refresh | Disabled animation for stability |

---

### Current State

- **Screen**: 284×76 landscape, black background, white text
- **Tasks**: 5 example tasks with different priorities
- **Navigation**: Rotary encoder with 2:1 ratio, task wrapping enabled
- **UI**: Home Screen with header/content/footer layout
- **Input**: Rotary encoder + button with full event detection
- **Performance**: Reliable scrolling without crashes

---

### Known Issues

- **Details Screen description scrolling not working**: Rotating the encoder in the Details Screen does not reliably scroll long descriptions. Observed behavior progressed from "no movement" (flex layout overriding manual `lv_obj_set_y`) to "scrolls slightly then snaps back to top" (layout pass re-clamping scroll on refresh). Current code uses a non-flex container with a content-sized label and elastic/momentum disabled, but scrolling still does not hold. Needs a deeper dive — likely the `forceScreenRefresh()`/`lv_refr_now()` path readjusting scroll, or the label content height not registering a real scrollable range.
- **Screen Animation Disabled**: LVGL animation system causes crashes when combined with screen refresh. Can be revisited later with a more sophisticated animation implementation.
- **Debug Output**: Serial debugging enabled at 115200 baud - can be disabled for production

---

### Remaining Work

- Fix Details Screen description scrolling (deep dive on LVGL scroll vs. refresh interaction)
- Implement smooth scroll animation without crashes
- Add task completion (double-click)
- Implement Bluetooth sync with mobile app
- Remove serial debugging for production build

---

## v0_display — TFT_eSPI Migration (Previous)

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