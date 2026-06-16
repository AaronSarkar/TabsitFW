# 001_system_architecture.md

# System Architecture

## Components

Application
↓
BLE Transport
↓
Communication Layer
↓
Task Service
↓
UI Layer
↓
Display Driver

---

# Module Responsibilities

## main

System initialization.

Responsibilities:

* Boot sequence
* Dependency wiring
* Event loop

---

## comms

Responsibilities:

* BLE connection
* Message parsing
* Sync handling

Outputs:

* Events

---

## models

Responsibilities:

* Domain entities

Contains:

* Task
* Device state

---

## storage

Responsibilities:

* Persist cached state

Implementation:

* ESP32 NVS

---

## input

Responsibilities:

* Encoder decoding
* Event emission

Outputs:

* Scroll
* Select
* Back
* Complete

---

## ui

Responsibilities:

* LVGL screens
* Navigation
* Rendering

---

## display

Responsibilities:

* Hardware abstraction
* Frame updates

---

# Event Flow

Encoder
→ Input
→ UI

BLE
→ Comms
→ Models
→ UI

Storage
→ Models
→ UI
