#ifndef DIAG_H
#define DIAG_H

// ============================================================
// Structured serial diagnostics for live PlatformIO monitor.
//
// Every line is prefixed with a tag so you can filter output:
//   [ENC]  - raw encoder GPIO / debounce
//   [EVT]  - input event queue (enqueue / dequeue)
//   [UI]   - UI state machine (screen, task index, anim flag)
//   [ANIM] - animation lifecycle (start / ready / stuck check)
//   [DISP] - LVGL display flush (count + area)
//   [HEAP] - LVGL heap snapshot
//   [LOOP] - main-loop heartbeat (every 2 s)
//
// Set DIAG_LEVEL to control verbosity:
//   0 = off, 1 = key events only, 2 = full trace
// ============================================================

#ifndef DIAG_LEVEL
#define DIAG_LEVEL 2
#endif

#include <Arduino.h>
#include <lvgl.h>

#if DIAG_LEVEL >= 1
  #define DIAG1(tag, fmt, ...) Serial.printf("[" tag "] " fmt "\n", ##__VA_ARGS__)
#else
  #define DIAG1(tag, fmt, ...) ((void)0)
#endif

#if DIAG_LEVEL >= 2
  #define DIAG2(tag, fmt, ...) Serial.printf("[" tag "] " fmt "\n", ##__VA_ARGS__)
#else
  #define DIAG2(tag, fmt, ...) ((void)0)
#endif

// Snapshot of LVGL heap (call anywhere)
inline void diagHeap(const char* label) {
#if DIAG_LEVEL >= 1
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Serial.printf("[HEAP] %s: free=%u used=%u frag=%u%%\n",
                label, (unsigned)mon.free_size,
                (unsigned)mon.total_size - (unsigned)mon.free_size,
                (unsigned)mon.frag_pct);
#else
  (void)label;
#endif
}

#endif // DIAG_H
