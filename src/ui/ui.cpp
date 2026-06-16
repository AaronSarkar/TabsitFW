#include "ui.h"
#include "../models/task.h"
#include "../display/lvgl_display.h"
#include <string.h>

namespace {
  // Screen objects
  lv_obj_t* screen_home = nullptr;
  lv_obj_t* screen_details = nullptr;
  
  // Home screen components
  lv_obj_t* home_header = nullptr;
  lv_obj_t* home_priority_label = nullptr;
  lv_obj_t* home_position_label = nullptr;
  lv_obj_t* home_content = nullptr;
  lv_obj_t* home_title_label = nullptr;
  lv_obj_t* home_footer = nullptr;
  lv_obj_t* home_due_label = nullptr;
  
  // Details screen components
  lv_obj_t* details_back_btn = nullptr;
  lv_obj_t* details_back_label = nullptr;
  lv_obj_t* details_content = nullptr;
  lv_obj_t* details_title_label = nullptr;
  lv_obj_t* details_desc_label = nullptr;
  
  // State
  UIState currentState = UI_HOME;
  int currentTaskIndex = 0;
  int totalTasks = 0;
  int rotationCount = 0; // Counter for encoder ratio
  constexpr int ENCODER_RATIO = 2; // Skip every other rotation
  
  // Animation objects
  lv_anim_t scroll_anim;
  bool animating = false;
  unsigned long animationStartTime = 0;
  int32_t scroll_offset = 0; // Current scroll offset for animation
  constexpr unsigned long ANIMATION_DURATION = 300; // Slower 300ms animation
}

// Forward declarations
static void setTimeout(void (*callback)(), unsigned long ms);
static void reset_anim_flag();

// Scroll animation callback for smooth task transitions
static void scroll_anim_cb(void* var, int32_t value) {
  scroll_offset = value;
  
  // Animate opacity for subtle fade effect (fade to 50% and back)
  lv_opa_t opacity = (lv_opa_t)value;
  
  if (home_content) {
    lv_obj_set_style_opa(home_content, opacity, 0);
  }
  if (home_header) {
    lv_obj_set_style_opa(home_header, opacity, 0);
  }
  if (home_footer) {
    lv_obj_set_style_opa(home_footer, opacity, 0);
  }
}

static void scroll_anim_ready_cb(lv_anim_t* a) {
  animating = false;
  scroll_offset = 0;
  
  // Reset opacity after animation
  if (home_content) {
    lv_obj_set_style_opa(home_content, LV_OPA_COVER, 0);
  }
  if (home_header) {
    lv_obj_set_style_opa(home_header, LV_OPA_COVER, 0);
  }
  if (home_footer) {
    lv_obj_set_style_opa(home_footer, LV_OPA_COVER, 0);
  }
}

// Simple animation callback for edge resistance
static void anim_y_cb(void* var, int32_t v) {
  lv_obj_set_y((lv_obj_t*)var, v);
}

static void reset_anim_flag() {
  animating = false;
}

static void startEdgeResistanceAnimation(int direction) {
  if (animating) return;
  
  animating = true;
  animationStartTime = millis();
  
  lv_anim_init(&scroll_anim);
  lv_anim_set_exec_cb(&scroll_anim, anim_y_cb);
  lv_anim_set_time(&scroll_anim, 120); // 120ms for edge resistance
  lv_anim_set_path_cb(&scroll_anim, lv_anim_path_ease_out);
  
  // Animate screen content slightly in the direction of rotation
  int32_t displacement = direction > 0 ? -40 : 40; // 40px resistance
  
  lv_anim_set_var(&scroll_anim, home_content);
  lv_anim_set_values(&scroll_anim, 0, displacement);
  lv_anim_start(&scroll_anim);
  
  // Return animation
  lv_anim_set_values(&scroll_anim, displacement, 0);
  lv_anim_set_delay(&scroll_anim, 120);
  lv_anim_start(&scroll_anim);
  
  // Reset animating flag after animation completes
  setTimeout(reset_anim_flag, 240);
}

static void startScrollAnimation(int direction) {
  // Don't block - allow new scrolls during animation
  scroll_offset = 0;
  
  lv_anim_init(&scroll_anim);
  lv_anim_set_exec_cb(&scroll_anim, scroll_anim_cb);
  lv_anim_set_time(&scroll_anim, ANIMATION_DURATION / 2); // 150ms each phase
  lv_anim_set_path_cb(&scroll_anim, lv_anim_path_ease_in_out);
  lv_anim_set_ready_cb(&scroll_anim, scroll_anim_ready_cb);
  
  // Fade from full opacity to 70% (subtle effect)
  lv_anim_set_values(&scroll_anim, LV_OPA_COVER, LV_OPA_70);
  lv_anim_start(&scroll_anim);
  
  // Then fade back to full opacity
  lv_anim_set_values(&scroll_anim, LV_OPA_70, LV_OPA_COVER);
  lv_anim_set_delay(&scroll_anim, ANIMATION_DURATION / 2);
  lv_anim_start(&scroll_anim);
}

// Simple timeout implementation using LVGL timer
static lv_timer_t* timeout_timer = nullptr;
static void (*timeout_callback)() = nullptr;

static void timeout_cb(lv_timer_t* timer) {
  if (timeout_callback) timeout_callback();
  lv_timer_del(timer);
  timeout_timer = nullptr;
  timeout_callback = nullptr;
}

static void setTimeout(void (*callback)(), unsigned long ms) {
  if (timeout_timer) {
    lv_timer_del(timeout_timer);
    timeout_timer = nullptr;
  }
  timeout_callback = callback;
  timeout_timer = lv_timer_create(timeout_cb, ms, nullptr);
}

static const char* getPriorityString(uint8_t priority) {
  switch (priority) {
    case PRIORITY_LOW: return "LOW";
    case PRIORITY_MEDIUM: return "MEDIUM";
    case PRIORITY_HIGH: return "HIGH";
    default: return "LOW";
  }
}

static lv_color_t getPriorityColor(uint8_t priority) {
  switch (priority) {
    case PRIORITY_LOW: return lv_color_hex(0x4CAF50);    // Green
    case PRIORITY_MEDIUM: return lv_color_hex(0xFF9800); // Orange
    case PRIORITY_HIGH: return lv_color_hex(0xF44336);   // Red
    default: return lv_color_hex(0x4CAF50);
  }
}

static const char* getDueDateString(uint64_t dueDate) {
  if (dueDate == 0) return "No Due Date";
  
  unsigned long currentTime = millis();
  unsigned long dueTime = dueDate / 1000; // Convert ms to seconds
  
  // Simple calculation (in real implementation, use proper time functions)
  long diff = dueTime - (currentTime / 1000);
  
  if (diff < 86400) return "Today";
  if (diff < 172800) return "Tomorrow";
  return "Upcoming";
}

static void createHomeScreen() {
  Serial.println("createHomeScreen called");
  
  screen_home = lv_obj_create(nullptr);
  if (!screen_home) {
    Serial.println("ERROR: Failed to create home screen");
    return;
  }
  Serial.println("Home screen created");
  
  lv_obj_set_size(screen_home, 284, 76);
  lv_obj_set_style_bg_color(screen_home, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(screen_home, 0, 0);
  lv_obj_set_style_pad_all(screen_home, 0, 0);
  
  // Header (18px height)
  home_header = lv_obj_create(screen_home);
  lv_obj_set_size(home_header, 284, 18);
  lv_obj_set_pos(home_header, 0, 0);
  lv_obj_set_style_bg_color(home_header, lv_color_hex(0x1A1A1A), 0);
  lv_obj_set_style_border_width(home_header, 0, 0);
  lv_obj_set_style_pad_all(home_header, 4, 0);
  
  home_priority_label = lv_label_create(home_header);
  lv_obj_set_style_text_font(home_priority_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(home_priority_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(home_priority_label, LV_ALIGN_LEFT_MID, 0, 0);
  
  home_position_label = lv_label_create(home_header);
  lv_obj_set_style_text_font(home_position_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(home_position_label, lv_color_hex(0x888888), 0);
  lv_obj_align(home_position_label, LV_ALIGN_RIGHT_MID, 0, 0);
  
  // Content (40px height)
  home_content = lv_obj_create(screen_home);
  lv_obj_set_size(home_content, 284, 40);
  lv_obj_set_pos(home_content, 0, 18);
  lv_obj_set_style_bg_color(home_content, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(home_content, 0, 0);
  lv_obj_set_style_pad_all(home_content, 8, 0);
  
  home_title_label = lv_label_create(home_content);
  lv_obj_set_style_text_font(home_title_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(home_title_label, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_long_mode(home_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(home_title_label, 268);
  lv_obj_align(home_title_label, LV_ALIGN_TOP_MID, 0, 0);
  
  // Footer (18px height)
  home_footer = lv_obj_create(screen_home);
  lv_obj_set_size(home_footer, 284, 18);
  lv_obj_set_pos(home_footer, 0, 58);
  lv_obj_set_style_bg_color(home_footer, lv_color_hex(0x1A1A1A), 0);
  lv_obj_set_style_border_width(home_footer, 0, 0);
  lv_obj_set_style_pad_all(home_footer, 4, 0);
  
  home_due_label = lv_label_create(home_footer);
  lv_obj_set_style_text_font(home_due_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(home_due_label, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(home_due_label, LV_ALIGN_CENTER, 0, 0);
  
  Serial.println("Home screen components created");
}

static void createDetailsScreen() {
  screen_details = lv_obj_create(nullptr);
  lv_obj_set_size(screen_details, 284, 76);
  lv_obj_set_style_bg_color(screen_details, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(screen_details, 0, 0);
  lv_obj_set_style_pad_all(screen_details, 0, 0);
  
  // Back button (18px height)
  details_back_btn = lv_obj_create(screen_details);
  lv_obj_set_size(details_back_btn, 60, 18);
  lv_obj_set_pos(details_back_btn, 0, 0);
  lv_obj_set_style_bg_color(details_back_btn, lv_color_hex(0x2A2A2A), 0);
  lv_obj_set_style_border_width(details_back_btn, 0, 0);
  lv_obj_set_style_radius(details_back_btn, 4, 0);
  lv_obj_set_style_pad_all(details_back_btn, 4, 0);
  
  details_back_label = lv_label_create(details_back_btn);
  lv_label_set_text(details_back_label, "← Back");
  lv_obj_set_style_text_font(details_back_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(details_back_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(details_back_label, LV_ALIGN_CENTER, 0, 0);
  
  // Content area (58px height)
  details_content = lv_obj_create(screen_details);
  lv_obj_set_size(details_content, 284, 58);
  lv_obj_set_pos(details_content, 0, 18);
  lv_obj_set_style_bg_color(details_content, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(details_content, 0, 0);
  lv_obj_set_style_pad_all(details_content, 8, 0);
  lv_obj_set_layout(details_content, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(details_content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(details_content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  details_title_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_title_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(details_title_label, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_long_mode(details_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(details_title_label, 268);
  
  details_desc_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_desc_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(details_desc_label, lv_color_hex(0xCCCCCC), 0);
  lv_label_set_long_mode(details_desc_label, LV_LABEL_LONG_SCROLL);
  lv_obj_set_width(details_desc_label, 268);
  lv_obj_set_height(details_desc_label, 30);
}

static void updateHomeScreen() {
  Serial.println("updateHomeScreen called");
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    Serial.println("No task found");
    lv_label_set_text(home_title_label, "No Tasks");
    lv_label_set_text(home_priority_label, "");
    lv_label_set_text(home_position_label, "");
    lv_label_set_text(home_due_label, "Open App to Sync");
    
    // Mark all labels as dirty to force redraw
    lv_obj_invalidate(home_title_label);
    lv_obj_invalidate(home_priority_label);
    lv_obj_invalidate(home_position_label);
    lv_obj_invalidate(home_due_label);
    // Also mark the screen as dirty
    lv_obj_invalidate(screen_home);
    return;
  }
  
  Serial.printf("Updating with task: %s\n", task->title.c_str());
  
  // Update header
  lv_label_set_text(home_priority_label, getPriorityString(task->priority));
  lv_obj_set_style_text_color(home_priority_label, getPriorityColor(task->priority), 0);
  lv_obj_invalidate(home_priority_label);
  
  char posStr[16];
  snprintf(posStr, sizeof(posStr), "%d/%d", currentTaskIndex + 1, totalTasks);
  lv_label_set_text(home_position_label, posStr);
  lv_obj_invalidate(home_position_label);
  
  // Update content
  lv_label_set_text(home_title_label, task->title.c_str());
  lv_obj_invalidate(home_title_label);
  
  // Update footer
  lv_label_set_text(home_due_label, getDueDateString(task->dueDate));
  lv_obj_invalidate(home_due_label);
  
  // Mark entire screen as dirty
  lv_obj_invalidate(screen_home);
  
  Serial.println("Screen update complete");
}

static void updateDetailsScreen() {
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    lv_label_set_text(details_title_label, "No Task");
    lv_label_set_text(details_desc_label, "");
    return;
  }
  
  lv_label_set_text(details_title_label, task->title.c_str());
  lv_label_set_text(details_desc_label, task->description.c_str());
}

void initUI() {
  Serial.println("initUI called");
  totalTasks = getTaskCount();
  Serial.printf("Total tasks: %d\n", totalTasks);
  
  createHomeScreen();
  createDetailsScreen();
  
  Serial.println("Loading home screen");
  lv_scr_load(screen_home);
  Serial.printf("Active screen after load: %p\n", lv_scr_act());
  Serial.printf("Home screen pointer: %p\n", screen_home);
  
  updateHomeScreen();
  
  // Force immediate redraw
  lv_refr_now(NULL);
  Serial.println("UI initialization complete");
}

void updateUI() {
  Serial.printf("updateUI: state=%d\n", currentState);
  if (currentState == UI_HOME) {
    updateHomeScreen();
  } else if (currentState == UI_DETAILS) {
    updateDetailsScreen();
  }
}

void handleInputEvent(InputEvent event) {
  Serial.printf("handleInputEvent: %d, state: %d, task: %d/%d\n", 
                event, currentState, currentTaskIndex, totalTasks);
  
  if (currentState == UI_HOME) {
    switch (event) {
      case ROTATE_NEXT:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          Serial.println("ROTATE_NEXT handling");
          
          // Update task index with wrapping
          currentTaskIndex++;
          if (currentTaskIndex >= totalTasks) {
            currentTaskIndex = 0; // Wrap to first task
          }
          Serial.printf("Index updated to: %d\n", currentTaskIndex);
          
          // Update screen content directly without animation to avoid crash
          updateHomeScreen();
          forceScreenRefresh();
          Serial.println("Display refreshed");
        } else {
          Serial.printf("Skipping rotation %d/%d\n", rotationCount, ENCODER_RATIO);
        }
        break;
      case ROTATE_PREV:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          Serial.println("ROTATE_PREV handling");
          
          // Update task index with wrapping
          currentTaskIndex--;
          if (currentTaskIndex < 0) {
            currentTaskIndex = totalTasks - 1; // Wrap to last task
          }
          Serial.printf("Index updated to: %d\n", currentTaskIndex);
          
          // Update screen content directly without animation to avoid crash
          updateHomeScreen();
          forceScreenRefresh();
          Serial.println("Display refreshed");
        } else {
          Serial.printf("Skipping rotation %d/%d\n", rotationCount, ENCODER_RATIO);
        }
        break;
      case CLICK:
        Serial.println("CLICK - switching to details");
        setUIState(UI_DETAILS);
        break;
      case DOUBLE_CLICK:
        Serial.println("DOUBLE_CLICK");
        break;
      case LONG_PRESS:
        Serial.println("LONG_PRESS - no action");
        break;
      default:
        Serial.printf("Unknown event: %d\n", event);
        break;
    }
  } else if (currentState == UI_DETAILS) {
    switch (event) {
      case CLICK:
        Serial.println("CLICK in details");
        break;
      case LONG_PRESS:
        Serial.println("LONG_PRESS - back to home");
        setUIState(UI_HOME);
        break;
      case ROTATE_NEXT:
      case ROTATE_PREV:
        Serial.println("ROTATE in details");
        break;
      default:
        Serial.printf("Unknown event in details: %d\n", event);
        break;
    }
  }
}

void setUIState(UIState state) {
  Serial.printf("setUIState: %d -> %d\n", currentState, state);
  currentState = state;
  if (state == UI_HOME) {
    Serial.println("Loading home screen");
    lv_scr_load(screen_home);
    Serial.printf("Active screen: %p\n", lv_scr_act());
    Serial.printf("Home screen: %p\n", screen_home);
    updateHomeScreen();
    lv_refr_now(NULL); // Force immediate redraw
  } else if (state == UI_DETAILS) {
    Serial.println("Loading details screen");
    lv_scr_load(screen_details);
    updateDetailsScreen();
    lv_refr_now(NULL); // Force immediate redraw
  }
  forceScreenRefresh();
}

UIState getUIState() {
  return currentState;
}

void setCurrentTaskIndex(int index) {
  if (index >= 0 && index < totalTasks) {
    currentTaskIndex = index;
  }
}

int getCurrentTaskIndex() {
  return currentTaskIndex;
}

void refreshTaskDisplay() {
  Serial.println("refreshTaskDisplay called");
  updateUI();
  forceScreenRefresh();
}