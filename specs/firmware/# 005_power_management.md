# 005_power_management.md

# Firmware Power Management Specification

Version: 1.0
Status: Draft

Depends On:

* 001_device_state_machine.md
* 003_ui_spec.md
* 004_input_spec.md

---

# Purpose

Defines:

* battery monitoring
* power states
* sleep behavior
* wake behavior
* display power management

Goals:

* maximize battery life
* preserve user experience
* maintain fast wake response

---

# Hardware Assumptions

MCU:
Xiao ESP32-C3

Battery:
Single-cell LiPo

Nominal Voltage:
3.7V

Full:
4.2V

Empty:
3.2V

Display:
ST7789

Motor:
3.3V coin vibration motor

---

# Design Principles

1. Screen consumes most power
2. Radio consumes second most power
3. Sleep aggressively
4. Wake instantly
5. Preserve state

---

# Power States

```cpp
enum PowerState {

    ACTIVE,

    IDLE,

    LIGHT_SLEEP,

    DEEP_SLEEP
};
```

---

# ACTIVE

Description:

Normal operation.

Enabled:

* display
* BLE
* encoder
* haptics

Conditions:

* user interacting
* sync active
* animation active

Target Duration:

minimum necessary

---

# IDLE

Entry Condition:

30 seconds
without interaction

Behavior:

* dim display
* reduce refresh rate

Enabled:

* BLE
* encoder

Display Brightness:

25%

Refresh Rate:

10 FPS

Exit:

user interaction

sync request

---

# LIGHT_SLEEP

Entry Condition:

5 minutes
without interaction

Behavior:

display off

BLE advertising reduced

CPU sleep allowed

Enabled:

* encoder wake
* BLE wake

Disabled:

* animations
* haptics

Preserved:

* RAM
* UI state
* task cache

Wake Target:

< 250 ms

---

# DEEP_SLEEP

Entry Condition:

30 minutes
without interaction

Behavior:

display off

BLE off

CPU halted

Motor off

Wake Sources:

encoder switch

charger connected

Preserved:

* NVS
* task cache

Lost:

* BLE session
* RAM state

Wake Target:

< 2 seconds

---

# Sleep State Flow

ACTIVE

↓

IDLE

↓

LIGHT_SLEEP

↓

DEEP_SLEEP

User interaction

↓

ACTIVE

Sync activity

↓

ACTIVE

---

# Display Power

ACTIVE

Brightness:

100%

IDLE

Brightness:

25%

LIGHT_SLEEP

Display off

DEEP_SLEEP

Display off

Rules:

Never redraw hidden screen.

Never animate in sleep.

---

# BLE Behavior

ACTIVE

Connected normally.

IDLE

Connected normally.

LIGHT_SLEEP

Advertising interval increased.

Reconnect allowed.

DEEP_SLEEP

BLE disabled.

Reconnect after wake.

---

# Battery Monitoring

Update Rate:

30 seconds

Source:

ADC

Moving Average:

5 samples

Purpose:

prevent voltage spikes from affecting reading

---

# Battery Levels

```cpp
enum BatteryLevel {

    FULL,

    HIGH,

    MEDIUM,

    LOW,

    CRITICAL
};
```

---

# Thresholds

FULL

4.0V+

HIGH

3.8V+

MEDIUM

3.6V+

LOW

3.4V+

CRITICAL

<3.3V

---

# Low Battery Behavior

LOW

Display warning icon.

No restrictions.

---

# Critical Battery Behavior

CRITICAL

Actions:

disable haptics

disable animations

save state

force sleep

Display:

Low Battery

Charge Device

---

# Charging

Detection:

charger present

Behavior:

prevent deep sleep

allow BLE

allow sync

Display:

charging indicator

---

# Wake Sources

Encoder Rotation

↓

ACTIVE

Encoder Click

↓

ACTIVE

BLE Sync Request

↓

ACTIVE

USB Connection

↓

ACTIVE

---

# Wake Flow

Wake Event

↓

Restore UI State

↓

Restore Selected Task

↓

Reconnect BLE

↓

Resume Operation

---

# State Persistence

Before Deep Sleep:

save:

* selected task index
* UI state
* last sync timestamp

Do Not Save:

* animation state
* BLE state
* temporary buffers

---

# Haptics Power Rules

ACTIVE

enabled

IDLE

enabled

LIGHT_SLEEP

disabled

DEEP_SLEEP

disabled

---

# Performance Targets

ACTIVE Current:

as low as practical

Wake Time:

LIGHT_SLEEP
<250 ms

DEEP_SLEEP
<2 s

Battery Monitoring:

<1% CPU usage

---

# Debug Logs

Examples:

```text
[POWER] ACTIVE

[POWER] IDLE

[POWER] LIGHT_SLEEP

[POWER] DEEP_SLEEP

[BATTERY] 3.82V

[WAKE] ENCODER
```

---

# Testing

Validate:

idle transition

sleep transition

deep sleep recovery

charging behavior

low battery warning

critical battery shutdown

BLE reconnect

state restoration

Pass Criteria:

No task loss

No corrupted storage

Wake always restores
previous selection
