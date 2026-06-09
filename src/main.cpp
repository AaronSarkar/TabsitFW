#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

constexpr int NUM_LEDS = sizeof(LEDs) / sizeof(LEDs[0]);

namespace {
  unsigned long lastTime = 0;
  int count = 0;
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NUM_LEDS; i++) pinMode(LEDs[i], OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);
  initEncoder();
}

void loop() {
  if (digitalRead(BUTTON) == 0) resetEncoder();

  updateEncoder();

  int rawCount = getEncoderRawCount();
  int ratio = getEncoderRatio();

  if (rawCount >= sq(NUM_LEDS) * ratio) {
    rawCount = 0;
    setEncoderRawCount(rawCount);
  }
  if (rawCount < 0) {
    rawCount = sq(NUM_LEDS) * ratio - 1;
    setEncoderRawCount(rawCount);
  }

  count = getEncoderCount();

  int rem = count;
  for (int j = 0; j < NUM_LEDS; j++) {
    digitalWrite(LEDs[j], rem & 1);
    rem >>= 1;
  }
  Serial.print(count);
  Serial.print(',');
  Serial.println(rawCount);
}
