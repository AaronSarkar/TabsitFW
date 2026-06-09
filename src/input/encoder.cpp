#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"

namespace {
  constexpr int ENCODER_RATIO = 2;
  int rawCount = 0;
  int encoderAPrev = 0;
  int encoderAValue = 0;
  EncoderDirection direction = ENCODER_NONE;
}

void initEncoder() {
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  encoderAPrev = digitalRead(ENCODER_A);
}

void updateEncoder() {
  direction = ENCODER_NONE;
  encoderAValue = digitalRead(ENCODER_A);

  if (encoderAValue != encoderAPrev) {
    if (digitalRead(ENCODER_B) != encoderAValue) {
      rawCount++;
      direction = ENCODER_CW;
    } else {
      rawCount--;
      direction = ENCODER_CCW;
    }
    encoderAPrev = encoderAValue;
  }
}

int getEncoderRawCount() {
  return rawCount;
}

void setEncoderRawCount(int value) {
  rawCount = value;
}

void resetEncoder() {
  rawCount = 0;
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
