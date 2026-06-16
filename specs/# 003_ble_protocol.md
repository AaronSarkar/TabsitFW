# 004_ble_protocol.md

# BLE Protocol Specification

Version: 1.0
Status: Draft

---

# Purpose

Defines communication between:

Mobile App
↔
Phone Case Firmware

This protocol is responsible for:

* Device pairing
* Task synchronization
* Completion events
* Connection recovery
* Local state reconciliation

---

# Roles

Mobile App:
BLE Central

Responsibilities:

* initiate connection
* push task updates
* reconcile state

Phone Case:
BLE Peripheral

Responsibilities:

* expose services
* store tasks
* notify task actions

---

# Transport

BLE GATT

Connection:

Phone
→ connect

Case
→ advertise

Security:

* bonded
* encrypted
* authenticated

No anonymous operation.

---

# Device Identity

Fields:

```json
{
  "deviceId":"case_001",
  "firmwareVersion":"1.0.0",
  "protocolVersion":1
}
```

Rules:

deviceId:
immutable

protocolVersion:
must match app support

---

# GATT Layout

Service:

TASK_SYNC_SERVICE

UUID:

```text
83A00000-0000-4000-8000-000000000001
```

Characteristics:

```text
83A00001
DEVICE_INFO

83A00002
SYNC_CONTROL

83A00003
TASK_DATA

83A00004
TASK_ACTIONS

83A00005
DEVICE_EVENTS
```

---

# Characteristic Definitions

## DEVICE_INFO

Direction:

Read

Purpose:

Device metadata.

Example:

```json
{
  "firmware":"1.0.0",
  "protocol":1,
  "battery":87
}
```

---

## SYNC_CONTROL

Direction:

Write + Notify

Purpose:

Coordinate sync lifecycle.

Commands:

```json
{
  "cmd":"BEGIN_SYNC"
}
```

```json
{
  "cmd":"END_SYNC"
}
```

```json
{
  "cmd":"REQUEST_RESYNC"
}
```

Responses:

```json
{
  "status":"OK"
}
```

```json
{
  "status":"ERROR"
}
```

---

## TASK_DATA

Direction:

Write

Purpose:

Transfer task snapshots.

Payload:

```json
{
  "type":"TASK",
  "task":{
      "...":"..."
  }
}
```

Behavior:

App sends tasks sequentially.

Case stores incrementally.

No full buffering.

---

## TASK_ACTIONS

Direction:

Notify

Purpose:

Device → App updates.

Supported:

```json
{
 "action":"COMPLETE",
 "taskId":"uuid"
}
```

Future:

```json
{
 "action":"OPEN"
}
```

---

## DEVICE_EVENTS

Direction:

Notify

Purpose:

Device status.

Events:

```json
{
 "event":"CONNECTED"
}
```

```json
{
 "event":"LOW_BATTERY"
}
```

```json
{
 "event":"SYNC_COMPLETE"
}
```

---

# Sync Lifecycle

Sequence:

App connects

↓

Read DEVICE_INFO

↓

BEGIN_SYNC

↓

Send tasks

↓

END_SYNC

↓

Case validates

↓

Persist

↓

SYNC_COMPLETE

---

# Sync Rules

App:

source of truth

Device:

cached mirror

Device may:

* complete task

Device may not:

* edit task content

Conflict resolution:

latest updatedAt wins

---

# Completion Flow

User:

double click encoder

↓

task.completed=true

↓

emit action

↓

app acknowledges

↓

save locally

Example:

```json
{
 "action":"COMPLETE",
 "taskId":"abc123",
 "updatedAt":1828328172
}
```

---

# Disconnect Handling

Unexpected disconnect:

retain local cache

Reconnect:

request incremental sync

Timeout:

10 seconds

Retries:

3

---

# Persistence

Store:

tasks.db
lastSyncTime
deviceId

Do not store:

BLE session keys
connection state

---

# Validation

Reject:

* unsupported protocol
* malformed JSON
* oversized payload
* unknown command

Error:

```json
{
 "status":"INVALID_PACKET"
}
```

---

# Payload Limits

Maximum characteristic payload:

180 bytes

If exceeded:

fragment packet

Example:

```json
{
 "chunk":1,
 "total":4,
 "data":"..."
}
```

---

# Logging

Examples:

```text
[BLE] Connected

[SYNC] Begin

[TASK] Updated

[STORE] Saved

[BLE] Disconnected
```

---

# Future Extensions

Reserved:

OTA updates

notification scheduling

multiple devices

encrypted task payloads
