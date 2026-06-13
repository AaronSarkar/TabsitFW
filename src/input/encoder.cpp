#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

namespace {
  constexpr int ENCODER_RATIO = 2;
  int rawCount = 0;
  int encoderAPrev = 0;
  int encoderAValue = 0;
  EncoderDirection direction = ENCODER_NONE;
  bool saturated = false;
}

bool initEncoder() {
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  encoderAPrev = digitalRead(ENCODER_A);

  int pinA = digitalRead(ENCODER_A);
  int pinB = digitalRead(ENCODER_B);
  if (pinA != LOW && pinA != HIGH) {
    Serial.println("[encoder] ERROR: ENCODER_A pin read returned unexpected value");
    return false;
  }
  if (pinB != LOW && pinB != HIGH) {
    Serial.println("[encoder] ERROR: ENCODER_B pin read returned unexpected value");
    return false;
  }

  return true;
}

void updateEncoder() {
  direction = ENCODER_NONE;
  encoderAValue = digitalRead(ENCODER_A);

  if (encoderAValue != encoderAPrev) {
    if (digitalRead(ENCODER_B) != encoderAValue) {
      if (rawCount < ENCODER_COUNT_MAX) {
        rawCount++;
      } else {
        saturated = true;
      }
      direction = ENCODER_CW;
    } else {
      if (rawCount > ENCODER_COUNT_MIN) {
        rawCount--;
      } else {
        saturated = true;
      }
      direction = ENCODER_CCW;
    }
    encoderAPrev = encoderAValue;
  }
}

int getEncoderRawCount() {
  return rawCount;
}

bool setEncoderRawCount(int value) {
  if (value < ENCODER_COUNT_MIN || value > ENCODER_COUNT_MAX) {
    Serial.print("[encoder] WARNING: setEncoderRawCount value out of range: ");
    Serial.println(value);
    return false;
  }
  rawCount = value;
  saturated = false;
  return true;
}

void resetEncoder() {
  rawCount = 0;
  saturated = false;
}

EncoderDirection getEncoderDirection() {
  return direction;
}

int getEncoderCount() {
  return rawCount / ENCODER_RATIO;
}

int getEncoderRatio() {
  return ENCODER_RATIO;
}

bool isEncoderSaturated() {
  return saturated;
}
