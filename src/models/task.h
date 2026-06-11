#ifndef TASK_H
#define TASK_H

#include <Arduino.h>

enum class Priority : uint8_t {
  Low,
  Medium,
  High,
  Critical
};

struct Task {
  uint8_t id;
  char title[32];
  char description[64];
  Priority priority;
  bool completed;
  uint32_t created_at;
  uint32_t due_at;
};

constexpr size_t MAX_TASKS = 20;

const Task* getTask(size_t index);
size_t getTaskCount();

#endif
