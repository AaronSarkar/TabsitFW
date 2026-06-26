#include "display.h"
#include "../diag.h"

namespace {
  TFT_eSPI* tft_instance = nullptr;
  lv_disp_draw_buf_t draw_buf;
  
  // Try full screen buffer instead of partial
  lv_color_t buf1[284 * 76];  // Full screen buffer
  lv_disp_drv_t disp_drv;
  lv_indev_drv_t indev_drv;

  // CGRAM offsets to center the 76x284 visible area in the 240x320 physical
  // panel, for rotation 1 (landscape). These used to be patched into TFT_eSPI's
  // ST7789_Rotation.h, but files under .pio/libdeps are wiped whenever
  // PlatformIO re-downloads the library, which silently broke the display.
  // Applying the offset here in our own flush keeps the fix permanent.
  constexpr uint16_t DISP_COLSTART = 18;
  constexpr uint16_t DISP_ROWSTART = 82;
}

static void disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  if (!tft_instance) {
    DIAG1("DISP", "Flush: TFT null!");
    lv_disp_flush_ready(disp);
    return;
  }

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // NOTE: no per-flush logging here — it runs every frame and throttles the
  // achievable frame rate during scrolling/animation.

  tft_instance->startWrite();
  tft_instance->setAddrWindow(area->x1 + DISP_COLSTART, area->y1 + DISP_ROWSTART, w, h);
  tft_instance->pushColors((uint16_t*)&color_p->full, w * h, true);
  tft_instance->endWrite();

  lv_disp_flush_ready(disp);
}

void initDisplay(TFT_eSPI* tft) {
  tft_instance = tft;

  lv_init();
  DIAG1("DISP", "LVGL init OK");
  
  // Use single buffer mode for simplicity
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, sizeof(buf1) / sizeof(lv_color_t));
  DIAG1("DISP", "Draw buf: %u pixels (%u bytes)",
        (unsigned)(sizeof(buf1) / sizeof(lv_color_t)), (unsigned)sizeof(buf1));

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 284;
  disp_drv.ver_res = 76;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  // Use full refresh mode
  disp_drv.full_refresh = 1;
  lv_disp_drv_register(&disp_drv);
  DIAG1("DISP", "Driver registered hor=%d ver=%d", disp_drv.hor_res, disp_drv.ver_res);
}

void updateDisplay() {
  lv_timer_handler();
}

void forceScreenRefresh() {
  lv_obj_t* screen = lv_scr_act();
  if (screen) {
    lv_obj_invalidate(screen);
  }
  lv_refr_now(NULL);
}
