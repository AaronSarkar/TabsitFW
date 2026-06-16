# 002_storage_spec.md

# Firmware Storage Specification

Version: 1.0
Status: Draft

Depends On:

* ../../specs/002_task_model.md
* ../../specs/003_ble_protocol.md

---

# Purpose

Defines local persistence for the phone case firmware.

Storage exists to:

* cache tasks
* preserve UI continuity
* recover from disconnects
* survive power cycles

Storage is not the source of truth.

The mobile application remains authoritative.

---

# Storage Backend

Implementation:

ESP32 NVS
(Non-Volatile Storage)

Access Layer:

storage_manager

Files:

```text
src/
└── storage/
    ├── storage_manager.h
    ├── storage_manager.cpp
    ├── serializers.h
    └── serializers.cpp
```

Responsibilities:

* read
* write
* validate
* migrate
* clear

---

# Persistence Strategy

Writes are explicit.

Never save continuously.

Allowed save triggers:

* sync completed
* task completion
* settings changed
* graceful shutdown

Forbidden:

* every encoder movement
* every render frame

---

# Data Model

Persisted State:

```cpp
struct PersistedState {

    uint8_t schemaVersion;

    uint64_t lastSyncTimestamp;

    uint16_t selectedTaskIndex;

    std::vector<Task> tasks;

};
```

---

# Namespace

NVS Namespace:

```text
device
```

---

# Keys

```text
tasks
sync_ts
selection
schema
```

Descriptions:

tasks
→ serialized task cache

sync_ts
→ last successful sync

selection
→ selected task

schema
→ storage version

---

# Serialization

Format:

JSON

Reason:

* easier debugging
* simpler migrations

Example:

{
"schema":1,
"tasks":[...]
}

Future:

CBOR
(MessagePack acceptable)

---

# Limits

Maximum tasks:

100

Maximum serialized payload:

100 KB

Maximum description:

500 chars

Target usage:

< 150 KB flash

---

# Boot Sequence

BOOT

↓

load schema

↓

validate

↓

load tasks

↓

restore selection

↓

UI ready

If invalid:

↓

factory reset storage

↓

request sync

---

# Save Sequence

event

↓

serialize

↓

validate

↓

write temp

↓

commit

↓

update timestamp

---

# Recovery

Corruption detection:

* invalid JSON
* schema mismatch
* deserialization failure

Recovery:

clear cache

request full sync

show reconnect state

No partial recovery.

---

# Task Updates

When task received:

validate

↓

replace existing

↓

mark dirty

↓

save after sync complete

Batch writes preferred.

---

# Completion Flow

double click

↓

mark completed

↓

save locally

↓

emit BLE action

↓

await confirmation

↓

clear dirty

If timeout:

retry

---

# Dirty Tracking

Track unsynced changes.

State:

```cpp
enum SyncState {
    CLEAN,
    DIRTY
};
```

Rules:

task completion
→ DIRTY

sync success
→ CLEAN

---

# Versioning

Storage schema:

1

Upgrade path:

1 → 2

Migration:

attempt

↓

fallback clear

---

# Factory Reset

Clear:

* tasks
* sync timestamp
* selection

Preserve:

* device ID
* firmware version

Trigger:

encoder held 10 s

OR

BLE command

---

# Memory Constraints

Heap target:

< 100 KB

Boot restore:

< 300 ms

Single save:

< 100 ms

---

# Testing

Validate:

* cold boot
* corrupted storage
* power loss during save
* 100 task restore
* reconnect sync
* migration path

Pass Criteria:

0 task loss
after normal shutdown

recover
after corruption
