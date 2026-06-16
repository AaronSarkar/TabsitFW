#ifndef TASK_H
#define TASK_H

#include <Arduino.h>

enum Priority : uint8_t {
  PRIORITY_LOW = 0,
  PRIORITY_MEDIUM = 1,
  PRIORITY_HIGH = 2
};

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

constexpr size_t MAX_TASKS = 100;

const Task* getTask(size_t index);
size_t getTaskCount();

#endif
