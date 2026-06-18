#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display/display.h"
#include "input/encoder.h"
#include "ui/ui.h"
#include "models/task.h"
#include "diag.h"

#define PIN_BL 21

TFT_eSPI tft;

void setup() {
  Serial.begin(115200);
  // USB-CDC serial: never block the main loop waiting for a host to read.
  // Without this, on a cold power-up with no serial monitor attached the TX
  // buffer fills and every Serial.printf() blocks, stalling the UI loop and
  // making scrolling extremely delayed/broken. A 0ms timeout drops output
  // instead of blocking when nothing is draining the port.
  Serial.setTxTimeoutMs(0);
  delay(1000);
  DIAG1("LOOP", "=== Tabsit booting ===");

  // Initialize backlight
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, LOW);

  // Initialize display
  tft.begin();
  tft.setRotation(1);
  DIAG1("LOOP", "Display: %dx%d", tft.width(), tft.height());

  // Initialize LVGL with TFT_eSPI
  initDisplay(&tft);

  // Initialize input manager
  initInputManager();
  DIAG1("LOOP", "Input manager ready");

  // Initialize UI
  initUI();
  diagHeap("post-initUI");

  // Force LVGL to refresh the screen immediately
  forceScreenRefresh();

  // Print task list
  int n = (int)getTaskCount();
  DIAG1("LOOP", "Tasks loaded: %d", n);
  for (int i = 0; i < n; i++) {
    const Task* task = getTask(i);
    if (task) {
      DIAG1("LOOP", "  [%d] \"%s\" pri=%d", i, task->title.c_str(), task->priority);
    }
  }

  DIAG1("LOOP", "=== Setup complete ===");
}

void loop() {
  // Heartbeat every 2 s so you can see the device is alive
  static unsigned long lastHeartbeat = 0;
  unsigned long now = millis();
  if (now - lastHeartbeat >= 2000) {
    lastHeartbeat = now;
    DIAG1("LOOP", "alive t=%lums idx=%d state=%d animFlag=%d",
          now, getCurrentTaskIndex(), (int)getUIState(), 0);
    diagHeap("heartbeat");
  }

  // Update LVGL display
  updateDisplay();

  // Update input manager
  updateInputManager();

  // Process input events
  InputEvent event;
  while (getNextInputEvent(&event)) {
    if (event != INPUT_NONE) {
      handleInputEvent(event);
    }
  }

  // Small delay to prevent CPU overload. Keep it short so lv_timer_handler()
  // is serviced finely enough to hit the ~60 FPS refresh period during
  // scrolling/animation.
  delay(2);
}
