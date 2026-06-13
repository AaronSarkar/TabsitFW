#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  pinMode(BUTTON, INPUT_PULLUP);
  initEncoder();
}

void loop() {
  if (digitalRead(BUTTON) == 0) resetEncoder();
  updateEncoder();

#ifdef DEBUG
  Serial.print(getEncoderCount());
  Serial.print(',');
  Serial.println(getEncoderRawCount());
#endif
}
