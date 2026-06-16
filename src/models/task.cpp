#include "task.h"

// Example tasks for development - matching spec format
const Task EXAMPLE_TASKS[] = {
  {
    .id = "8c57d645-48c7-4db1-9a44-82b5b6ec72f1",
    .title = "Submit Firmware Spec",
    .description = "Complete the firmware specification document including UI, input, and task model details. Review with team and finalize. Ensure all technical requirements are met and architecture diagrams are included.",
    .priority = PRIORITY_HIGH,
    .dueDate = 1717507200000ULL, // Tomorrow
    .completed = false,
    .updatedAt = 1717420800000ULL,
    .version = 1
  },
  {
    .id = "a1b2c3d4-5e6f-7g8h-9i0j-1k2l3m4n5o6p",
    .title = "Buy Groceries",
    .description = "Milk, eggs, bread, butter, and vegetables for the week. Check the pantry first to avoid duplicates. Don't forget to buy coffee beans and orange juice for breakfast.",
    .priority = PRIORITY_MEDIUM,
    .dueDate = 1717593600000ULL, // In 2 days
    .completed = false,
    .updatedAt = 1717420800000ULL,
    .version = 1
  },
  {
    .id = "b2c3d4e5-6f7g-8h-9i0j-1k2l3m4n5o6p7q",
    .title = "Walk the Dog",
    .description = "30 min walk in the park. Remember to bring waste bags. If the weather is nice, extend to 45 minutes and visit the dog park. Bring water for both of you.",
    .priority = PRIORITY_LOW,
    .dueDate = 1717420800000ULL, // Today
    .completed = false,
    .updatedAt = 1717420800000ULL,
    .version = 1
  },
  {
    .id = "c3d4e5f6-7g8h-9i0j-1k2l-3m4n5o6p7q8r",
    .title = "Read Chapter 5",
    .description = "Embedded Systems book pages 120-150. Take notes on memory management techniques. Pay special attention to the heap and stack sections. Prepare questions for the next class discussion.",
    .priority = PRIORITY_LOW,
    .dueDate = 0, // No due date
    .completed = false,
    .updatedAt = 1717420800000ULL,
    .version = 1
  },
  {
    .id = "d4e5f6g7-8h9i-0j1k-2l3m4n5o6p7q8r9s",
    .title = "Fix Leaking Tap",
    .description = "Call plumber or fix with wrench. Need to purchase replacement washer first. Turn off water supply under the sink before starting. Have towels ready for any water spills during the repair.",
    .priority = PRIORITY_HIGH,
    .dueDate = 1717514400000ULL, // Tomorrow
    .completed = false,
    .updatedAt = 1717420800000ULL,
    .version = 1
  },
};

constexpr size_t EXAMPLE_TASK_COUNT = sizeof(EXAMPLE_TASKS) / sizeof(EXAMPLE_TASKS[0]);

const Task* getTask(size_t index) {
  if (index >= EXAMPLE_TASK_COUNT) return nullptr;
  return &EXAMPLE_TASKS[index];
}

size_t getTaskCount() {
  return EXAMPLE_TASK_COUNT;
}