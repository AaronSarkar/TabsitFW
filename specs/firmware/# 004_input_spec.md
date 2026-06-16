# 004_input_spec.md

# Firmware Input Specification

Version: 1.0
Status: Draft

Depends On:

* 003_ui_spec.md
* 001_device_state_machine.md

---

# Purpose

Defines:

* encoder behavior
* click handling
* event generation
* navigation interaction
* haptic triggers

Input should feel:

* immediate
* predictable
* tactile

Input events are translated into actions.

UI must never directly read hardware.

---

# Hardware

Input Device:

KY-040 Rotary Encoder

Signals:

CLK
DT
SW

Mode:

interrupt driven

Pullups:

enabled

---

# Architecture

Layers:

```text id="tv4vzk"
Hardware
↓

Encoder Driver

↓

Input Manager

↓

Input Events

↓

UI Manager
```

Responsibilities:

Encoder Driver:
decode rotation

Input Manager:
interpret intent

UI:
consume events

---

# Input Event Model

```cpp id="lx5n1n"
enum InputEvent {

    ROTATE_NEXT,

    ROTATE_PREV,

    CLICK,

    DOUBLE_CLICK,

    LONG_PRESS,

    RELEASE
};
```

Events are queued.

Never processed inside ISR.

---

# Queue

Implementation:

ring buffer

Capacity:

16 events

Behavior:

overflow

↓

drop oldest

↓

log warning

---

# Rotation

Purpose:

navigate tasks

Rules:

1 detent

↓

1 task movement

No acceleration.

No momentum.

No skipping.

---

Rotation Timing

Minimum interval:

40 ms

Events inside interval:

ignored

Purpose:

prevent accidental jumps.

---

HOME Behavior

Clockwise

↓

next task

↓

play transition

Counterclockwise

↓

previous task

↓

play transition

If animation active:

queue latest direction

discard intermediate.

Example:

CW CW CW

↓

move once

↓

animate

↓

move final position

---

DETAILS Behavior

Rotate

↓

scroll description

Scroll speed:

12 px

No paging.

---

Edge Behavior

At first task:

rotate back

↓

resistance haptic

↓

no movement

At last task:

same behavior

---

Click Behavior

Detection Window:

250 ms

Debounce:

20 ms

---

Single Click

HOME:

open details

DETAILS:

toggle scroll lock

Animation:

120 ms

Motor:

10 ms pulse

---

Double Click

HOME only

Action:

complete task

Sequence:

mark complete

↓

play completion animation

↓

send BLE update

↓

persist

Timeout:

500 ms

Motor:

100 ms pulse

Ignore:

while syncing

---

Long Press

Threshold:

700 ms

Behavior:

DETAILS

↓

return HOME

HOME

↓

no action

No repeat.

Motor:

40 ms pulse

---

Invalid Inputs

Ignore:

rotation during completion

click during transition

double click in details

long press during sync

---

Input Lock States

```cpp id="13p5jw"
enum InputLock {

    NONE,

    ANIMATING,

    SYNCING,

    ERROR
};
```

Behavior:

NONE

all input

ANIMATING

rotation limited

SYNCING

navigation only

ERROR

long press only

---

Interrupt Model

ISR Responsibilities:

read pins

enqueue raw state

return

ISR Budget:

<100 μs

Forbidden:

BLE

LVGL

malloc

logging

---

Haptics

Rotate:

none

Edge resistance:

10 ms

Open details:

10 ms

Complete:

100 ms

Back:

40 ms

Error:

40 ms × 2

---

Power Rules

Wake Sources:

encoder click

rotation

Motor disabled:

during sleep

---

Debug Logs

Examples:

```text id="c5gk8l"
[INPUT] ROTATE_NEXT

[INPUT] CLICK

[INPUT] COMPLETE

[INPUT] LONG_PRESS
```

---

Testing

Validate:

slow rotation

fast rotation

rapid direction changes

double click

bounce

wake from sleep

queue overflow

Pass Criteria:

0 missed inputs

No accidental completion
