# Fonts

UI uses the rounded **Quicksand** typeface (OFL), converted to LVGL C arrays.

Generated files live in `src/ui/fonts/` and are declared in `src/ui/ui.cpp`:

- `font_quicksand_18` — Quicksand SemiBold (600), 18px — titles / hero text
- `font_quicksand_13` — Quicksand Medium (500), 13px — secondary / body text

## Regenerate

From `tools/` (installs `lv_font_conv` locally):

```
npm install
```

Then from the project root:

```
tools/node_modules/.bin/lv_font_conv --font tools/fonts/Quicksand-600.ttf --size 18 --bpp 4 \
  --format lvgl --no-compress --force-fast-kern-format --lv-include lvgl.h \
  -r 0x20-0x7E -o src/ui/fonts/font_quicksand_18.c

tools/node_modules/.bin/lv_font_conv --font tools/fonts/Quicksand-500.ttf --size 13 --bpp 4 \
  --format lvgl --no-compress --force-fast-kern-format --lv-include lvgl.h \
  -r 0x20-0x7E -o src/ui/fonts/font_quicksand_13.c
```

Note: the generated files set `.user_data`, so `LV_USE_USER_DATA 1` must stay enabled in `include/lv_conf.h`.
