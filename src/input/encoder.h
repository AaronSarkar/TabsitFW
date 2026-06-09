#ifndef ENCODER_H
#define ENCODER_H

enum EncoderDirection {
  ENCODER_NONE,
  ENCODER_CW,
  ENCODER_CCW
};

void initEncoder();
void updateEncoder();
int getEncoderRawCount();
void setEncoderRawCount(int value);
void resetEncoder();
EncoderDirection getEncoderDirection();
int getEncoderCount();
int getEncoderRatio();

#endif
