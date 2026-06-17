#include <Arduino.h>
#include "config/pins.h"
#include "input/encoder.h"
#include "diag.h"

namespace {
  constexpr int QUEUE_SIZE = 16;
  constexpr unsigned long ROTATION_DEBOUNCE_MS = 40;
  constexpr unsigned long CLICK_DEBOUNCE_MS = 20;
  constexpr unsigned long CLICK_WINDOW_MS = 250;
  constexpr unsigned long DOUBLE_CLICK_TIMEOUT_MS = 500;
  constexpr unsigned long LONG_PRESS_THRESHOLD_MS = 700;

  InputEvent eventQueue[QUEUE_SIZE];
  int queueHead = 0;
  int queueTail = 0;
  InputLock currentLock = LOCK_NONE;

  // Encoder state
  int encoderAPrev = HIGH;
  unsigned long lastRotationTime = 0;

  // Button state
  int buttonPrev = HIGH;
  unsigned long buttonPressTime = 0;
  unsigned long lastClickTime = 0;
  unsigned long clickDebounceTime = 0;
  bool buttonPressed = false;
  bool longPressTriggered = false;
  bool clickPending = false;
  bool clickDebouncing = false;
}

void initInputManager() {
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  
  encoderAPrev = digitalRead(ENCODER_A);
  buttonPrev = digitalRead(BUTTON);
}

bool enqueueEvent(InputEvent event) {
  int nextHead = (queueHead + 1) % QUEUE_SIZE;
  if (nextHead == queueTail) {
    // Queue full, drop oldest
    queueTail = (queueTail + 1) % QUEUE_SIZE;
    DIAG1("EVT", "Queue full - dropped oldest event");
  }
  eventQueue[queueHead] = event;
  queueHead = nextHead;
  DIAG2("EVT", "Enqueued event=%d  qHead=%d qTail=%d depth=%d",
        event, queueHead, queueTail,
        (queueHead - queueTail + QUEUE_SIZE) % QUEUE_SIZE);
  return true;
}

bool getNextInputEvent(InputEvent* event) {
  if (queueTail == queueHead) {
    *event = INPUT_NONE;
    return false;
  }
  *event = eventQueue[queueTail];
  queueTail = (queueTail + 1) % QUEUE_SIZE;
  DIAG2("EVT", "Dequeued event=%d  qHead=%d qTail=%d", *event, queueHead, queueTail);
  return true;
}

void setInputLock(InputLock lock) {
  currentLock = lock;
}

InputLock getInputLock() {
  return currentLock;
}

void clearInputQueue() {
  queueHead = 0;
  queueTail = 0;
}

void updateInputManager() {
  unsigned long currentTime = millis();

  // Handle encoder rotation
  int encoderA = digitalRead(ENCODER_A);
  if (encoderA != encoderAPrev) {
    DIAG2("ENC", "A edge: A=%d B=%d dt=%lums lock=%d",
          encoderA, digitalRead(ENCODER_B),
          currentTime - lastRotationTime, currentLock);
    if ((currentTime - lastRotationTime) >= ROTATION_DEBOUNCE_MS) {
      int encoderB = digitalRead(ENCODER_B);
      if (encoderB != encoderA) {
        if (currentLock == LOCK_NONE || currentLock == LOCK_ANIMATING) {
          enqueueEvent(ROTATE_NEXT);
          DIAG1("ENC", "ROTATE_NEXT  A=%d B=%d", encoderA, encoderB);
        } else {
          DIAG1("ENC", "ROTATE_NEXT blocked by lock=%d", currentLock);
        }
      } else {
        if (currentLock == LOCK_NONE || currentLock == LOCK_ANIMATING) {
          enqueueEvent(ROTATE_PREV);
          DIAG1("ENC", "ROTATE_PREV  A=%d B=%d", encoderA, encoderB);
        } else {
          DIAG1("ENC", "ROTATE_PREV blocked by lock=%d", currentLock);
        }
      }
      lastRotationTime = currentTime;
    } else {
      DIAG2("ENC", "Debounced (dt=%lums < %dms)", currentTime - lastRotationTime, ROTATION_DEBOUNCE_MS);
    }
    encoderAPrev = encoderA;
  }

  // Handle button
  int button = digitalRead(BUTTON);
  
  if (button == LOW && buttonPrev == HIGH) {
    // Button pressed
    buttonPressTime = currentTime;
    buttonPressed = true;
    longPressTriggered = false;
    clickDebouncing = false;
    DIAG1("ENC", "Button pressed t=%lu", currentTime);
  } 
  else if (button == HIGH && buttonPrev == LOW) {
    // Button released
    if (buttonPressed) {
      if (!longPressTriggered) {
        // Check for double click
        if ((currentTime - lastClickTime) < DOUBLE_CLICK_TIMEOUT_MS && clickPending) {
          if (currentLock == LOCK_NONE) {
            enqueueEvent(DOUBLE_CLICK);
            DIAG1("ENC", "DOUBLE_CLICK");
          }
          clickPending = false;
          clickDebouncing = false;
        } else {
          // Potential single click - start debounce
          clickPending = true;
          lastClickTime = currentTime;
          clickDebounceTime = currentTime;
          clickDebouncing = true;
          DIAG2("ENC", "Click debouncing...");
        }
      }
      enqueueEvent(RELEASE);
      DIAG2("ENC", "Button released held=%lums", currentTime - buttonPressTime);
    }
    buttonPressed = false;
    longPressTriggered = false;
  }
  
  // Handle click debounce (non-blocking)
  if (clickDebouncing && (currentTime - clickDebounceTime) >= CLICK_DEBOUNCE_MS) {
    if (clickPending) {
      if (currentLock == LOCK_NONE || currentLock == LOCK_SYNCING) {
        enqueueEvent(CLICK);
        DIAG1("ENC", "CLICK confirmed");
      } else {
        DIAG1("ENC", "CLICK blocked by lock=%d", currentLock);
      }
      clickPending = false;
    }
    clickDebouncing = false;
  }
  
  // Check for long press
  if (buttonPressed && !longPressTriggered && (currentTime - buttonPressTime) >= LONG_PRESS_THRESHOLD_MS) {
    longPressTriggered = true;
    clickPending = false;
    clickDebouncing = false;
    if (currentLock == LOCK_NONE || currentLock == LOCK_ERROR) {
      enqueueEvent(LONG_PRESS);
      DIAG1("ENC", "LONG_PRESS");
    }
  }

  buttonPrev = button;
}
