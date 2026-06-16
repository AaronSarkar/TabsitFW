#include "lvgl_display.h"

namespace {
  TFT_eSPI* tft_instance = nullptr;
  lv_disp_draw_buf_t draw_buf;
  
  // Try full screen buffer instead of partial
  lv_color_t buf1[284 * 76];  // Full screen buffer
  lv_disp_drv_t disp_drv;
  lv_indev_drv_t indev_drv;
}

static void disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  static int flush_count = 0;
  flush_count++;
  
  Serial.printf("Flush #%d: area(%d,%d,%d,%d) size=%dx%d\n", 
                flush_count, area->x1, area->y1, area->x2, area->y2, 
                (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
  
  if (!tft_instance) {
    Serial.println("Flush: TFT instance null");
    return;
  }

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft_instance->startWrite();
  tft_instance->setAddrWindow(area->x1, area->y1, w, h);
  tft_instance->pushColors((uint16_t*)&color_p->full, w * h, true);
  tft_instance->endWrite();

  Serial.printf("Flush #%d: wrote %d pixels\n", flush_count, w * h);
  lv_disp_flush_ready(disp);
}

void initLvglDisplay(TFT_eSPI* tft) {
  Serial.println("initLvglDisplay called");
  tft_instance = tft;

  lv_init();
  Serial.println("LVGL initialized");
  
  // Use single buffer mode for simplicity
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, sizeof(buf1) / sizeof(lv_color_t));
  Serial.printf("Draw buffer initialized: %d pixels\n", sizeof(buf1) / sizeof(lv_color_t));

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 284;
  disp_drv.ver_res = 76;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  // Use full refresh mode
  disp_drv.full_refresh = 1;
  lv_disp_drv_register(&disp_drv);
  Serial.println("Display driver registered");
}

void updateLvglDisplay() {
  static unsigned long last_timer_debug = 0;
  unsigned long current_time = millis();
  
  // Debug every 5 seconds
  if (current_time - last_timer_debug >= 5000) {
    Serial.println("LVGL timer handler called");
    last_timer_debug = current_time;
  }
  
  lv_timer_handler();
}

void forceScreenRefresh() {
  Serial.println("Force screen refresh called");
  lv_obj_t* screen = lv_scr_act();
  if (screen) {
    lv_obj_invalidate(screen);
    Serial.println("Screen invalidated");
  }
  
  // Force LVGL to redraw immediately
  lv_refr_now(NULL);
  Serial.println("Force redraw called");
  
  lv_timer_handler();
  Serial.println("Timer handler called");
}