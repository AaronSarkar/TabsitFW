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
}

// Forward declarations

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
  
  // Content area (58px height) - scrollable vertically.
  // NOTE: deliberately NOT a flex layout. Flex re-runs on every refresh and
  // readjusts/clamps the scroll position, which fought the scrolling. With a
  // plain container + content-sized label, the scrollable area is stable.
  details_content = lv_obj_create(screen_details);
  lv_obj_set_size(details_content, 284, 58);
  lv_obj_set_pos(details_content, 0, 18);
  lv_obj_set_style_bg_color(details_content, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(details_content, 0, 0);
  lv_obj_set_style_pad_all(details_content, 8, 0);
  // Enable crisp vertical scrolling (no elastic bounce / momentum on a knob)
  lv_obj_set_scroll_dir(details_content, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(details_content, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_clear_flag(details_content, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_clear_flag(details_content, LV_OBJ_FLAG_SCROLL_MOMENTUM);
  
  details_title_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_title_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(details_title_label, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_long_mode(details_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(details_title_label, 268);
  lv_obj_set_pos(details_title_label, 0, 0);
  
  details_desc_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_desc_label, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(details_desc_label, lv_color_hex(0xCCCCCC), 0);
  lv_label_set_long_mode(details_desc_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(details_desc_label, 268);
  lv_obj_set_pos(details_desc_label, 0, 18);
  // Height defaults to LV_SIZE_CONTENT so the label grows to fit the full
  // wrapped text, giving the container a real scrollable range.
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
  
  // Reset scroll back to the top for the newly opened task
  lv_obj_scroll_to_y(details_content, 0, LV_ANIM_OFF);
  
  Serial.println("Details screen updated");
}

static void scrollDescription(int direction) {
  // Scroll the content container by 12px per detent. LVGL clamps the
  // scroll position to the content bounds automatically, so we don't
  // need to track/limit the offset ourselves.
  lv_obj_scroll_by(details_content, 0, -12 * direction, LV_ANIM_OFF);
  forceScreenRefresh();
  
  Serial.printf("Description scroll y: %d\n", lv_obj_get_scroll_y(details_content));
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
        Serial.println("ROTATE_NEXT in details - scroll down");
        scrollDescription(1);
        break;
      case ROTATE_PREV:
        Serial.println("ROTATE_PREV in details - scroll up");
        scrollDescription(-1);
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