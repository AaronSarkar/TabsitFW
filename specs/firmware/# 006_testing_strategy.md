# 006_testing_strategy.md

# Firmware Testing Strategy

Version: 1.0
Status: Draft

Depends On:

* 001_device_state_machine.md
* 002_storage_spec.md
* 003_ui_spec.md
* 004_input_spec.md
* 005_power_management.md
* ../../specs/002_task_model.md
* ../../specs/003_ble_protocol.md

---

# Purpose

Defines:

* testing philosophy
* validation requirements
* regression prevention
* acceptance criteria

The objective is to ensure firmware reliability while enabling rapid AI-assisted development.

---

# Testing Philosophy

Priority Order:

1. Prevent data loss
2. Prevent crashes
3. Prevent incorrect task state
4. Prevent BLE sync failures
5. Prevent UI regressions
6. Optimize performance

Every feature change must be testable.

No feature is complete without validation.

---

# Test Levels

```text
Unit Tests
    ↓
Integration Tests
    ↓
Hardware Tests
    ↓
Manual Validation
```

All four levels are required before release.

---

# Unit Tests

Purpose:

Validate isolated logic.

Targets:

* task model
* storage serialization
* BLE packet parsing
* event queues
* state transitions
* input handling

Must Not Require:

* display
* encoder hardware
* BLE hardware

---

# Task Model Tests

Validate:

* create task
* update task
* complete task
* delete task
* invalid task rejection
* sorting rules

Pass Criteria:

100% pass

---

# Storage Tests

Validate:

* serialize
* deserialize
* empty storage
* corrupted storage
* schema mismatch
* migration

Pass Criteria:

No task loss

---

# BLE Protocol Tests

Validate:

* BEGIN_SYNC
* END_SYNC
* COMPLETE action
* malformed packet
* unsupported version
* oversized packet

Pass Criteria:

Invalid packets rejected safely

---

# Input Tests

Validate:

* rotation
* debounce
* click
* double click
* long press
* queue overflow

Pass Criteria:

No duplicate events

---

# Integration Tests

Purpose:

Validate module interaction.

---

# Sync Flow Test

Scenario:

App connects

↓

Sync begins

↓

Tasks transferred

↓

Persisted

↓

Displayed

Pass Criteria:

Task visible after sync

---

# Completion Flow Test

Scenario:

Task selected

↓

Double click

↓

Completion event sent

↓

Task saved

↓

UI updated

Pass Criteria:

State consistent everywhere

---

# Reboot Recovery Test

Scenario:

Tasks stored

↓

Power cycle

↓

Restore

Pass Criteria:

Selected task restored

No corruption

---

# Hardware Tests

Purpose:

Validate behavior on real device.

Required Hardware:

* ESP32-C3
* ST7789 display
* KY-040 encoder
* vibration motor

---

# Display Tests

Validate:

* boot screen
* task rendering
* animation
* details screen
* empty state

Pass Criteria:

No flicker

No tearing

---

# Encoder Tests

Validate:

* slow rotation
* fast rotation
* direction reversal
* long press
* double click

Pass Criteria:

No missed inputs

---

# BLE Tests

Validate:

* first pairing
* reconnect
* disconnect recovery
* sync recovery

Pass Criteria:

No lockups

---

# Power Tests

Validate:

* idle timeout
* light sleep
* deep sleep
* wake restore

Pass Criteria:

Correct state restored

---

# Stress Tests

Purpose:

Identify long-term issues.

---

# Task Capacity Test

Load:

100 tasks

Validate:

scrolling

animation

storage

Pass Criteria:

Responsive UI

---

# Rapid Navigation Test

Rotate encoder continuously

Duration:

5 minutes

Pass Criteria:

No crashes

No memory growth

---

# Repeated Sync Test

Run:

100 sync cycles

Pass Criteria:

No corruption

---

# Long Runtime Test

Duration:

24 hours

Validate:

memory leaks

BLE stability

sleep behavior

Pass Criteria:

No reset

No crash

---

# Failure Injection

Purpose:

Validate recovery paths.

---

# Storage Corruption

Inject:

invalid JSON

Expected:

clear cache

request sync

---

# Unexpected Power Loss

Power removed during save

Expected:

boot successfully

recover state

---

# BLE Disconnect During Sync

Disconnect midway

Expected:

retry

request resync

---

# Memory Monitoring

Track:

heap free

largest block

task count

event queue depth

Log:

every 60 seconds

Pass Criteria:

Stable over time

---

# Performance Targets

Boot Time:

< 2 seconds

Task Switch Animation:

< 200 ms

Input Response:

< 50 ms

Storage Save:

< 100 ms

Wake From Light Sleep:

< 250 ms

Wake From Deep Sleep:

< 2 seconds

---

# Release Checklist

Before Release:

✓ Build passes

✓ Unit tests pass

✓ Integration tests pass

✓ Hardware validation completed

✓ Stress tests completed

✓ No critical bugs open

✓ Documentation updated

---

# AI Development Rules

Any AI-generated feature must:

1. Compile successfully
2. Add/update tests
3. Pass existing tests
4. Preserve shared contracts

Reject pull request if:

* tests removed
* shared protocol changed without spec update
* validation missing

---

# Definition of Done

A feature is complete only when:

* implementation finished
* tests added
* tests passing
* hardware validated
* documentation updated
