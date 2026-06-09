#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);
  initEncoder();
}

void loop() {
  if (digitalRead(BUTTON) == 0) resetEncoder();
  updateEncoder();

  Serial.print(getEncoderCount());
  Serial.print(',');
  Serial.println(getEncoderRawCount());
}
