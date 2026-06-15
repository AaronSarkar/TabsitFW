#include <Arduino.h>
#include <TFT_eSPI.h>

#define PIN_BL 21

TFT_eSPI tft;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("start");

  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, LOW);

  tft.begin();
  tft.setRotation(1);

  int w = tft.width();
  int h = tft.height();

  Serial.printf("w=%d h=%d\n", w, h);

  tft.fillScreen(TFT_NAVY);

  // Border - should be tight on all 4 edges if centered correctly
  tft.drawRect(0, 0, w, h, TFT_YELLOW);

  // Corner markers (6x6 squares)
  tft.fillRect(0, 0, 6, 6, TFT_RED);
  tft.fillRect(w-6, 0, 6, 6, TFT_GREEN);
  tft.fillRect(0, h-6, 6, 6, TFT_BLUE);
  tft.fillRect(w-6, h-6, 6, 6, TFT_WHITE);

  // Center crosshair
  int cx = w / 2;
  int cy = h / 2;
  tft.drawLine(cx - 20, cy, cx + 20, cy, TFT_CYAN);
  tft.drawLine(cx, cy - 10, cx, cy + 10, TFT_CYAN);

  // Original three lines using drawString (print/println with font 1 is broken)
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawString("Hello XIAO!", 4, 10, 2);
  tft.drawString("284x76 landscape", 4, 26, 2);
  tft.drawString("TFT_eSPI", 4, 42, 2);

  Serial.println("done");
}

void loop() {
  delay(5000);
}
