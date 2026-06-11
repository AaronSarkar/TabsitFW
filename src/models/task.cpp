#include "task.h"

Task createTask(uint8_t id, const char* title, const char* description,
                Priority priority, uint32_t due_at) {
  Task t;
  t.id = id;
  strncpy(t.title, title, sizeof(t.title) - 1);
  t.title[sizeof(t.title) - 1] = '\0';
  strncpy(t.description, description, sizeof(t.description) - 1);
  t.description[sizeof(t.description) - 1] = '\0';
  t.priority = priority;
  t.completed = false;
  t.created_at = millis();
  t.due_at = due_at;
  return t;
}

// Example tasks for development
const Task EXAMPLE_TASKS[] = {
  createTask(0, "Buy groceries", "Milk, eggs, bread, butter", Priority::High, 0),
  createTask(1, "Walk the dog", "30 min walk in the park", Priority::Medium, 0),
  createTask(2, "Read chapter 5", "Embedded Systems book pages 120-150", Priority::Low, 0),
  createTask(3, "Fix leaking tap", "Call plumber or fix with wrench", Priority::Critical, 0),
  createTask(4, "Water plants", "Indoor and balcony plants", Priority::Low, 0),
};

constexpr size_t EXAMPLE_TASK_COUNT = sizeof(EXAMPLE_TASKS) / sizeof(EXAMPLE_TASKS[0]);

const Task* getTask(size_t index) {
  if (index >= EXAMPLE_TASK_COUNT) return nullptr;
  return &EXAMPLE_TASKS[index];
}

size_t getTaskCount() {
  return EXAMPLE_TASK_COUNT;
}
