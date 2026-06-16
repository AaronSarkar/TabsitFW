#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "../models/task.h"
#include "../input/encoder.h"

enum UIState {
  UI_HOME,
  UI_DETAILS
};

void initUI();
void updateUI();
void handleInputEvent(InputEvent event);
void setUIState(UIState state);
UIState getUIState();
void setCurrentTaskIndex(int index);
int getCurrentTaskIndex();
void refreshTaskDisplay();

#endif
