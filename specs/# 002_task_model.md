# 003_task_model.md

# Task Model Specification

Version: 1.0
Status: Draft

---

# Purpose

Defines the canonical task representation shared between:

* Mobile App
* BLE Communication Layer
* Firmware
* Local Device Storage

This document is the source of truth for task-related data.

---

# Design Goals

1. Small memory footprint
2. Fast rendering
3. Stable synchronization
4. Forward compatibility
5. Offline support

---

# Constraints

Device constraints:

* ESP32-C3
* Limited RAM
* Limited persistent storage
* BLE bandwidth limitations

Therefore:

* Keep payloads compact
* Avoid nested objects
* Avoid variable schemas

---

# Task Entity

## Fields

```cpp
struct Task {
    String id;

    String title;

    String description;

    uint8_t priority;

    uint64_t dueDate;

    bool completed;

    uint64_t updatedAt;

    uint8_t version;
};
```

---

# Field Definitions

## id

Type:
String

Description:
Globally unique identifier.

Requirements:

* Generated on phone
* Immutable
* UUID v4 preferred

Example:

```text
8c57d645-48c7-4db1-9a44-82b5b6ec72f1
```

---

## title

Type:
String

Constraints:

* Required
* Maximum 60 characters

Displayed:

* Home Screen
* Details Screen

Examples:

```text
Prepare presentation
Pay hydro bill
```

---

## description

Type:
String

Constraints:

* Optional
* Maximum 500 characters

Displayed:

* Details only

Empty value:

```text
""
```

---

## priority

Type:
uint8

Enum:

```cpp
enum Priority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};
```

Rendering:

LOW:
small indicator

MEDIUM:
medium indicator

HIGH:
strong visual emphasis

---

## dueDate

Type:
Unix timestamp (UTC ms)

Optional

Value:

```text
0 = unset
```

Device behavior:

* No scheduling logic
* Display only

---

## completed

Type:
bool

Behavior:

false
→ active

true
→ completed

Completion source:

* App
* Device

---

## updatedAt

Type:
Unix timestamp (UTC ms)

Purpose:

Conflict resolution

Rules:

Newest update wins.

---

## version

Type:
uint8

Purpose:

Schema compatibility

Initial value:

```text
1
```

---

# Task Collection

Storage:

```cpp
std::vector<Task>
```

Maximum:

```text
100 tasks
```

Sort Order:

1. incomplete
2. priority descending
3. due date ascending
4. updatedAt descending

Example:

Task A
Task B
Task C

---

# Local Persistence

Saved:

* full task list
* selection index
* last sync timestamp

Not saved:

* screen animations
* BLE state

---

# Sync Rules

Phone → Device

Allowed:

* create
* update
* delete
* complete

Device → Phone

Allowed:

* complete
* selection metadata

Device cannot:

* create tasks
* edit titles
* edit descriptions

---

# Deletion

Soft delete.

Field:

```cpp
bool deleted
```

Rules:

deleted=true
→ hidden

Cleanup:

after successful sync

---

# Memory Budget

Estimated:

Per task:
~700 bytes worst case

100 tasks:
~70 KB

Target:
<100 KB total

---

# Validation

Reject task if:

* missing id
* title empty
* title > 60 chars
* description > 500 chars
* invalid priority
* version unsupported

---

# Future Extensions

Reserved:

* categories
* recurring tasks
* reminders
* tags
* attachments
