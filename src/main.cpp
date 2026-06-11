#include <Arduino.h>
#include "display/display.h"

Display tft;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("start");

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BLACK);

  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setCursor(4, 10);
  tft.println("Hello XIAO!");
  tft.setCursor(4, 26);
  tft.println("284x76 landscape");
  tft.setCursor(4, 42);
  tft.println("Custom driver");

  Serial.println("done");
}

void loop() {
  delay(5000);
}
