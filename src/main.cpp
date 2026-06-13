#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

namespace {
  bool encoderReady = false;
}

void setup() {
  Serial.begin(9600);

  unsigned long serialTimeout = millis() + 3000;
  while (!Serial && millis() < serialTimeout) {
    // wait up to 3 s for USB-CDC serial on native-USB boards
  }

  pinMode(BUTTON, INPUT_PULLUP);

  encoderReady = initEncoder();
  if (!encoderReady) {
    Serial.println("[main] ERROR: encoder initialization failed");
  }
}

void loop() {
  if (!encoderReady) {
    Serial.println("[main] ERROR: encoder not initialized, retrying...");
    delay(1000);
    encoderReady = initEncoder();
    return;
  }

  if (digitalRead(BUTTON) == 0) resetEncoder();
  updateEncoder();

  Serial.print(getEncoderCount());
  Serial.print(',');
  Serial.println(getEncoderRawCount());
}
