# 000_project_scope.md

# Project Scope

## Product Vision

A phone case with an integrated secondary display that helps users stay aware of tasks without opening their phone.

The device acts as a low-friction companion interface to a mobile task application.

Primary interaction goals:

* Surface today's tasks
* Allow lightweight navigation
* Allow quick completion
* Deliver subtle haptic reminders

This is not intended to replace the mobile application.

---

# Hardware

## MCU

Xiao ESP32-C3

## Display

76 × 284 TFT LCD
Driver: ST7789P3

## Input

KY-040 rotary encoder

* Rotate → navigate
* Single click → open details
* Double click → complete
* Long press → return

## Haptics

3.3V coin vibration motor

---

# Firmware Responsibilities

The firmware SHALL:

* Maintain BLE connection with phone
* Render current task state
* Cache tasks locally
* Respond to encoder input
* Trigger haptic notifications
* Recover gracefully from disconnects

The firmware SHALL NOT:

* Edit task content
* Store user accounts
* Connect directly to cloud APIs
* Perform scheduling logic

---

# Initial MVP

Included:

* BLE pairing
* Task sync
* Local persistence
* Task browsing
* Task completion
* Home screen
* Details screen

Excluded:

* Categories
* Analytics
* Widgets
* OTA updates
* Multiple users
* Cloud sync
* Authentication accounts

---

# Success Metrics

Cold boot:
< 2 seconds

Scroll latency:
< 50 ms

Task sync:
< 3 seconds

Battery:
Target > 1 day

Maximum tasks stored:
100
