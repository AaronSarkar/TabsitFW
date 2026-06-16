#ifndef ENCODER_H
#define ENCODER_H

enum InputEvent {
  INPUT_NONE,
  ROTATE_NEXT,
  ROTATE_PREV,
  CLICK,
  DOUBLE_CLICK,
  LONG_PRESS,
  RELEASE
};

enum InputLock {
  LOCK_NONE,
  LOCK_ANIMATING,
  LOCK_SYNCING,
  LOCK_ERROR
};

void initInputManager();
void updateInputManager();
bool getNextInputEvent(InputEvent* event);
void setInputLock(InputLock lock);
InputLock getInputLock();
void clearInputQueue();

#endif
