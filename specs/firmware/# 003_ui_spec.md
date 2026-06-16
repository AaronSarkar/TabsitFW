# 003_ui_spec.md

# Firmware UI Specification

Version: 2.0
Status: Draft

Depends On:

* ../../specs/002_task_model.md
* ../SPEC.md
* 001_device_state_machine.md

---

# Purpose

Defines:

* LVGL screen architecture
* navigation model
* animation behavior
* rendering constraints

The UI is designed to feel:

* glanceable
* tactile
* calm
* focused

Only one task is visible at a time.

---

# Display

Resolution:

284 × 76 px

Orientation:

Landscape

Framework:

LVGL 9.5

Driver:

TFT_eSPI

Target FPS:

30

Minimum:

20

---

# Design Principles

1. One task only
2. Motion communicates position
3. Details without clutter
4. Immediate interaction
5. Preserve focus

---

# Navigation Model

Encoder Rotate:

next / previous task

Single Click:

expand details

Double Click:

complete task

Long Press:

return

Navigation depth:

Maximum:
2

HOME

↓

DETAILS

---

# Screen Stack

```text
HOME
↓

DETAILS
```

Persistent screens only.

No dynamic allocation.

---

# UI State

```cpp
enum UIState {
    HOME,
    DETAILS
};
```

Persist:

selectedTaskIndex

Do not persist:

animation state

---

# HOME SCREEN

Purpose:

Focused task browsing.

One task visible.

---

Layout

```text
┌────────────────────────────┐
│ HIGH                 3/12  │
│ Submit Firmware Spec       │
│ Due Tomorrow               │
└────────────────────────────┘
```

---

Sections

Header
Task Content
Footer

---

Header

Height:
18 px

Left:

priority

Right:

task position

Example:

HIGH        4/12

---

Content

Height:
40 px

Display:

title only

Rules:

max 2 lines

ellipsis enabled

description hidden

---

Footer

Height:
18 px

Display:

due date

Examples:

Today

Tomorrow

No Due Date

---

Task Switching

Encoder Rotate

↓

animate transition

↓

change selectedTask

---

Animation

Direction:

Rotate Clockwise

↓

next task enters from bottom

↓

current task exits upward

Rotate Counterclockwise

↓

previous enters from top

↓

current exits downward

Motion:

vertical slide

---

Transition Timing

Duration:

180 ms

Easing:

ease-out

No bounce.

Interruptible:

YES

If encoder rotates repeatedly:

cancel current

continue latest

---

Animation Example

Current:

```text
┌────────────────────┐
│ Finish Report       │
└────────────────────┘
```

Rotate:

```text
Current
↑

Next
↑
```

Final:

```text
┌────────────────────┐
│ Grocery Shopping   │
└────────────────────┘
```

---

Edge Behavior

First task

↓

rotate backward

↓

small resistance animation

Last task

↓

rotate forward

↓

small resistance animation

No wrap.

Resistance:

40 px

Return:

120 ms

Motor:

optional 10 ms pulse

---

Empty State

Display:

```text
┌────────────────────┐
│                    │
│ No Tasks           │
│ Open App to Sync   │
│                    │
└────────────────────┘
```

No interaction.

---

Loading State

Display:

```text
┌────────────────────┐
│ Syncing            │
│                    │
│ • • •              │
└────────────────────┘
```

No screen transition.

---

Offline State

Display:

```text
┌────────────────────┐
│ Offline            │
│ Showing Cache      │
└────────────────────┘
```

Browsing remains enabled.

---

DETAILS SCREEN

Purpose:

Expanded reading.

Entry:

single click

Layout:

```text
┌────────────────────────────┐
│ ← Back                     │
│                            │
│ Submit Firmware Spec       │
│                            │
│ Long description text...   │
└────────────────────────────┘
```

---

Behavior

Encoder:

scroll details

Single Click:

pause scroll

Long Press:

return

Double Click:

disabled

---

Completion

HOME only.

Flow:

Double Click

↓

100 ms motor

↓

check animation

↓

send BLE update

↓

advance to next task

---

Completion Animation

Task

↓

compress

↓

fade

↓

slide upward

↓

next task enters

Duration:

250 ms

---

LVGL Architecture

Objects:

```cpp
lv_obj_t* screen_home;

lv_obj_t* card_current;

lv_obj_t* card_next;

lv_obj_t* screen_details;
```

Rules:

Exactly two cards.

Reuse card objects.

Never recreate.

---

Rendering Rules

Redraw only:

selection change

task update

screen change

Avoid:

full refresh

dynamic style creation

continuous animation loops

---

Theme

Background:

black

Text:

white

Accent:

priority only

Fonts:

2 sizes max

---

Testing

Validate:

rapid rotation

100 tasks

animation interruption

cold boot restore

completion transition

Pass Criteria:

No dropped frames

No visual tearing
