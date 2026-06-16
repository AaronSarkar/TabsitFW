#ifndef LVGL_DISPLAY_H
#define LVGL_DISPLAY_H

#include <lvgl.h>
#include <TFT_eSPI.h>

void initLvglDisplay(TFT_eSPI* tft);
void updateLvglDisplay();
void forceScreenRefresh();

#endif