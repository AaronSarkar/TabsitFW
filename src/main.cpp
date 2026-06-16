#include <Arduino.h>
#include <TFT_eSPI.h>
#include "display/lvgl_display.h"
#include "input/encoder.h"
#include "ui/ui.h"
#include "models/task.h"

// Forward declaration
void forceScreenRefresh();

#define PIN_BL 21

TFT_eSPI tft;
unsigned long lastUpdateTime = 0;
constexpr unsigned long UPDATE_INTERVAL = 16; // ~60 FPS

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing Task Display System...");

  // Initialize backlight
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, LOW);

  // Initialize display
  tft.begin();
  tft.setRotation(1);
  
  int w = tft.width();
  int h = tft.height();
  Serial.printf("Display: %dx%d\n", w, h);

  // Initialize LVGL with TFT_eSPI
  initLvglDisplay(&tft);
  Serial.println("LVGL initialized");

  // Initialize input manager
  initInputManager();
  Serial.println("Input manager initialized");

  // Initialize UI
  initUI();
  Serial.println("UI initialized");
  
  // Force LVGL to refresh the screen immediately
  Serial.println("Forcing initial LVGL refresh...");
  forceScreenRefresh();
  Serial.println("Initial refresh complete");

  // Print task information
  Serial.printf("Total tasks: %d\n", getTaskCount());
  for (int i = 0; i < getTaskCount(); i++) {
    const Task* task = getTask(i);
    if (task) {
      Serial.printf("Task %d: %s (Priority: %d)\n", i, task->title.c_str(), task->priority);
    }
  }

  Serial.println("Setup complete - System ready");
}

void loop() {
  unsigned long currentTime = millis();

  // Update LVGL display more frequently
  updateLvglDisplay();

  // Update input manager
  updateInputManager();

  // Process input events
  InputEvent event;
  while (getNextInputEvent(&event)) {
    if (event != INPUT_NONE) {
      Serial.printf("Processing event: %d\n", event);
      handleInputEvent(event);
    }
  }

  // Small delay to prevent CPU overload
  delay(5);
}
