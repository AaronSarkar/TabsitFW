#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>
#include <TFT_eSPI.h>

void initDisplay(TFT_eSPI* tft);
void updateDisplay();
void forceScreenRefresh();

#endif
