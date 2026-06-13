#ifndef ENCODER_H
#define ENCODER_H

#include <limits.h>

enum EncoderDirection {
  ENCODER_NONE,
  ENCODER_CW,
  ENCODER_CCW
};

static constexpr int ENCODER_COUNT_MAX =  INT_MAX / 2;
static constexpr int ENCODER_COUNT_MIN = -INT_MAX / 2;

bool initEncoder();
void updateEncoder();
int getEncoderRawCount();
bool setEncoderRawCount(int value);
void resetEncoder();
EncoderDirection getEncoderDirection();
int getEncoderCount();
int getEncoderRatio();
bool isEncoderSaturated();

#endif
