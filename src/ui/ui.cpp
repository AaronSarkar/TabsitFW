#include "ui.h"
#include "../models/task.h"
#include "../display/display.h"
#include "../diag.h"
#include <string.h>

// Custom rounded fonts (Quicksand, generated into src/ui/fonts/)
LV_FONT_DECLARE(font_quicksand_18);
LV_FONT_DECLARE(font_quicksand_13);
#define FONT_TITLE  (&font_quicksand_18)
#define FONT_SMALL  (&font_quicksand_13)

// ---- Palette: dark & muted, Gemini-inspired ----
#define COL_BG          lv_color_hex(0x16171A)  // single consistent background
#define COL_TEXT        lv_color_hex(0xE6E6E6)  // primary text
#define COL_TEXT_BODY   lv_color_hex(0xC2C5CA)  // body / description text
#define COL_TEXT_DIM    lv_color_hex(0x868B93)  // secondary / muted text
#define COL_PRIO_LOW    lv_color_hex(0x5BB974)  // soft green
#define COL_PRIO_MED    lv_color_hex(0xF9C84B)  // soft amber
#define COL_PRIO_HIGH   lv_color_hex(0xF28B82)  // soft red
// Gemini gradient stops (blue -> purple -> pink)
#define COL_GRAD_A      lv_color_hex(0x4893FF)
#define COL_GRAD_B      lv_color_hex(0x9B72CB)
#define COL_GRAD_C      lv_color_hex(0xE0719B)

namespace {
  // Screen objects
  lv_obj_t* screen_home = nullptr;
  lv_obj_t* screen_details = nullptr;
  
  // Home screen components
  lv_obj_t* home_priority_dot = nullptr;
  lv_obj_t* home_position_label = nullptr;
  lv_obj_t* home_title_label = nullptr;
  lv_obj_t* home_due_label = nullptr;
  lv_obj_t* home_accent = nullptr;
  
  // Details screen components
  lv_obj_t* details_content = nullptr;
  lv_obj_t* details_title_label = nullptr;
  lv_obj_t* details_desc_label = nullptr;
  lv_obj_t* details_accent = nullptr;

  // Shared Gemini gradient descriptor (must persist for the lifetime of styles)
  lv_grad_dsc_t gemini_grad;
  
  // State
  UIState currentState = UI_HOME;
  int currentTaskIndex = 0;
  int totalTasks = 0;
  int rotationCount = 0; // Counter for encoder ratio
  constexpr int ENCODER_RATIO = 2; // Skip every other rotation
  
  // Animation state
  bool animationInProgress = false;
}

// Forward declarations
static void titleFadeOutCallback(void* var, int32_t v);
static void titleFadeInCallback(void* var, int32_t v);
static void titleFadeOutReadyCb(lv_anim_t* a);
static void titleFadeInReadyCb(lv_anim_t* a);
static void cancelAnimation();
static void startTitleFadeAnimation();

static void initGeminiGradient() {
  lv_memset_00(&gemini_grad, sizeof(gemini_grad));
  gemini_grad.dir = LV_GRAD_DIR_HOR;
  gemini_grad.stops_count = 3;
  gemini_grad.stops[0].color = COL_GRAD_A; gemini_grad.stops[0].frac = 0;
  gemini_grad.stops[1].color = COL_GRAD_B; gemini_grad.stops[1].frac = 128;
  gemini_grad.stops[2].color = COL_GRAD_C; gemini_grad.stops[2].frac = 255;
}

// Thin rounded gradient accent bar pinned to the bottom edge.
static lv_obj_t* createAccentBar(lv_obj_t* parent) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_size(bar, 284, 3);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_set_style_pad_all(bar, 0, 0);
  lv_obj_set_style_shadow_width(bar, 0, 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(bar, COL_GRAD_A, 0);
  lv_obj_set_style_bg_grad(bar, &gemini_grad, 0);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
  return bar;
}

static lv_color_t getPriorityColor(uint8_t priority) {
  switch (priority) {
    case PRIORITY_LOW: return COL_PRIO_LOW;
    case PRIORITY_MEDIUM: return COL_PRIO_MED;
    case PRIORITY_HIGH: return COL_PRIO_HIGH;
    default: return COL_PRIO_LOW;
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

// Apply the consistent dark background + no border/padding to a screen root.
static void styleScreenRoot(lv_obj_t* scr) {
  lv_obj_set_size(scr, 284, 76);
  lv_obj_set_style_bg_color(scr, COL_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(scr, 0, 0);
  lv_obj_set_style_radius(scr, 0, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
}

static void createHomeScreen() {
  screen_home = lv_obj_create(nullptr);
  if (!screen_home) {
    Serial.println("ERROR: Failed to create home screen");
    return;
  }
  styleScreenRoot(screen_home);

  // Priority dot (top-left) — small colored circle, color set per task.
  home_priority_dot = lv_obj_create(screen_home);
  lv_obj_set_size(home_priority_dot, 9, 9);
  lv_obj_set_style_radius(home_priority_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(home_priority_dot, 0, 0);
  lv_obj_set_style_shadow_width(home_priority_dot, 0, 0);
  lv_obj_set_style_pad_all(home_priority_dot, 0, 0);
  lv_obj_set_style_bg_opa(home_priority_dot, LV_OPA_COVER, 0);
  lv_obj_clear_flag(home_priority_dot, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(home_priority_dot, LV_ALIGN_TOP_LEFT, 16, 12);

  // Position counter (top-right), muted.
  home_position_label = lv_label_create(screen_home);
  lv_obj_set_style_text_font(home_position_label, FONT_SMALL, 0);
  lv_obj_set_style_text_color(home_position_label, COL_TEXT_DIM, 0);
  lv_obj_align(home_position_label, LV_ALIGN_TOP_RIGHT, -16, 9);

  // Title (hero), rounded font, vertically centered.
  home_title_label = lv_label_create(screen_home);
  lv_obj_set_style_text_font(home_title_label, FONT_TITLE, 0);
  lv_obj_set_style_text_color(home_title_label, COL_TEXT, 0);
  lv_label_set_long_mode(home_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(home_title_label, 252);
  lv_obj_align(home_title_label, LV_ALIGN_LEFT_MID, 16, -1);

  // Due date (bottom-left), muted.
  home_due_label = lv_label_create(screen_home);
  lv_obj_set_style_text_font(home_due_label, FONT_SMALL, 0);
  lv_obj_set_style_text_color(home_due_label, COL_TEXT_DIM, 0);
  lv_obj_align(home_due_label, LV_ALIGN_BOTTOM_LEFT, 16, -9);

  // Gemini gradient accent line along the bottom edge.
  home_accent = createAccentBar(screen_home);
}

static void createDetailsScreen() {
  screen_details = lv_obj_create(nullptr);
  styleScreenRoot(screen_details);

  // Scrollable content area — consistent transparent background (no bars).
  // NOTE: deliberately NOT a flex layout. Flex re-runs on every refresh and
  // readjusts/clamps the scroll position, which fought the scrolling. With a
  // plain container + content-sized labels, the scrollable area is stable.
  details_content = lv_obj_create(screen_details);
  lv_obj_set_size(details_content, 284, 72);
  lv_obj_set_pos(details_content, 0, 0);
  lv_obj_set_style_bg_opa(details_content, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(details_content, 0, 0);
  lv_obj_set_style_radius(details_content, 0, 0);
  lv_obj_set_style_pad_left(details_content, 16, 0);
  lv_obj_set_style_pad_right(details_content, 16, 0);
  lv_obj_set_style_pad_top(details_content, 10, 0);
  lv_obj_set_style_pad_bottom(details_content, 8, 0);
  // Crisp vertical scrolling (no elastic bounce / momentum on a knob)
  lv_obj_set_scroll_dir(details_content, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(details_content, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(details_content, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_clear_flag(details_content, LV_OBJ_FLAG_SCROLL_MOMENTUM);

  details_title_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_title_label, FONT_TITLE, 0);
  lv_obj_set_style_text_color(details_title_label, COL_TEXT, 0);
  lv_label_set_long_mode(details_title_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(details_title_label, 252);
  lv_obj_set_pos(details_title_label, 0, 0);

  details_desc_label = lv_label_create(details_content);
  lv_obj_set_style_text_font(details_desc_label, FONT_SMALL, 0);
  lv_obj_set_style_text_color(details_desc_label, COL_TEXT_BODY, 0);
  lv_label_set_long_mode(details_desc_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(details_desc_label, 252);
  lv_obj_set_pos(details_desc_label, 0, 26);
  // Height defaults to LV_SIZE_CONTENT so the label grows to fit the full
  // wrapped text, giving the container a real scrollable range.

  // Gemini gradient accent line along the bottom edge.
  details_accent = createAccentBar(screen_details);
}

static void titleFadeOutCallback(void* var, int32_t v) {
  DIAG2("ANIM", "FadeOut opa=%d", v);
  lv_obj_set_style_opa(home_title_label, v, 0);
}

static void titleFadeInCallback(void* var, int32_t v) {
  DIAG2("ANIM", "FadeIn  opa=%d", v);
  lv_obj_set_style_opa(home_title_label, v, 0);
}

static void titleFadeOutReadyCb(lv_anim_t* a) {
  DIAG1("ANIM", "FadeOut READY -> updating content, idx=%d animFlag=%d",
        currentTaskIndex, (int)animationInProgress);
  
  // Update content with new task data
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    if (home_title_label) lv_label_set_text(home_title_label, "No Tasks");
    if (home_priority_dot) lv_obj_add_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
    if (home_position_label) lv_label_set_text(home_position_label, "");
    if (home_due_label) lv_label_set_text(home_due_label, "Open App to Sync");
  } else {
    // Priority dot color
    if (home_priority_dot) {
      lv_obj_clear_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_bg_color(home_priority_dot, getPriorityColor(task->priority), 0);
    }
    
    char posStr[16];
    snprintf(posStr, sizeof(posStr), "%d / %d", currentTaskIndex + 1, totalTasks);
    if (home_position_label) lv_label_set_text(home_position_label, posStr);
    
    // Update content
    if (home_title_label) lv_label_set_text(home_title_label, task->title.c_str());
    
    // Update footer
    if (home_due_label) lv_label_set_text(home_due_label, getDueDateString(task->dueDate));
  }
  
  // Start fade in animation.
  // lv_anim_start() copies the struct internally, so a stack-local is safe here.
  lv_anim_t fadeInAnim;
  lv_anim_init(&fadeInAnim);
  lv_anim_set_var(&fadeInAnim, home_title_label);
  lv_anim_set_values(&fadeInAnim, LV_OPA_0, LV_OPA_COVER);
  lv_anim_set_time(&fadeInAnim, 150);
  lv_anim_set_exec_cb(&fadeInAnim, titleFadeInCallback);
  lv_anim_set_ready_cb(&fadeInAnim, titleFadeInReadyCb);
  lv_anim_set_path_cb(&fadeInAnim, lv_anim_path_ease_out);
  lv_anim_start(&fadeInAnim);
  DIAG1("ANIM", "FadeIn started");
}

static void titleFadeInReadyCb(lv_anim_t* a) {
  animationInProgress = false;
  DIAG1("ANIM", "FadeIn READY -> animFlag cleared, idx=%d", currentTaskIndex);
  diagHeap("post-anim");
}

// Cancel any running fade animation and restore full opacity.
// Must be called before touching any LVGL label/object that the
// animation might currently own.
static void cancelAnimation() {
  if (!animationInProgress) return;
  lv_anim_del(home_title_label, titleFadeOutCallback);
  lv_anim_del(home_title_label, titleFadeInCallback);
  if (home_title_label) lv_obj_set_style_opa(home_title_label, LV_OPA_COVER, 0);
  animationInProgress = false;
  DIAG1("ANIM", "Cancelled - opacity restored");
}

static void startTitleFadeAnimation() {
  if (animationInProgress) {
    DIAG1("ANIM", "Already in progress - skip (idx=%d)", currentTaskIndex);
    return;
  }
  
  animationInProgress = true;
  DIAG1("ANIM", "FadeOut starting idx=%d", currentTaskIndex);
  diagHeap("pre-anim");
  
  // lv_anim_start() copies the struct internally, so a stack-local is safe here.
  lv_anim_t fadeOutAnim;
  lv_anim_init(&fadeOutAnim);
  lv_anim_set_var(&fadeOutAnim, home_title_label);
  lv_anim_set_values(&fadeOutAnim, LV_OPA_COVER, LV_OPA_0);
  lv_anim_set_time(&fadeOutAnim, 150);
  lv_anim_set_exec_cb(&fadeOutAnim, titleFadeOutCallback);
  lv_anim_set_ready_cb(&fadeOutAnim, titleFadeOutReadyCb);
  lv_anim_set_path_cb(&fadeOutAnim, lv_anim_path_ease_in);
  lv_anim_start(&fadeOutAnim);
  DIAG1("ANIM", "FadeOut started");
}

static void updateHomeScreen() {
  // NOTE: do NOT call cancelAnimation() here. This function is called from
  // titleFadeOutReadyCb (mid-animation) and must not disturb the animation
  // lifecycle. Callers that need to cancel first (e.g. setUIState) do so
  // themselves before calling this.
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    DIAG1("UI", "updateHomeScreen: no task at idx=%d", currentTaskIndex);
    lv_label_set_text(home_title_label, "No Tasks");
    lv_obj_add_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(home_position_label, "");
    lv_label_set_text(home_due_label, "Open App to Sync");
    lv_obj_invalidate(screen_home);
    return;
  }
  
  DIAG2("UI", "updateHomeScreen idx=%d title=\"%s\"", currentTaskIndex, task->title.c_str());
  
  // Priority dot color
  lv_obj_clear_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_color(home_priority_dot, getPriorityColor(task->priority), 0);
  
  char posStr[16];
  snprintf(posStr, sizeof(posStr), "%d / %d", currentTaskIndex + 1, totalTasks);
  lv_label_set_text(home_position_label, posStr);
  
  // Update content
  lv_label_set_text(home_title_label, task->title.c_str());
  
  // Update footer
  lv_label_set_text(home_due_label, getDueDateString(task->dueDate));
  
  // Mark entire screen as dirty
  lv_obj_invalidate(screen_home);
}

static void updateDetailsScreen() {
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    DIAG1("UI", "updateDetailsScreen: no task at idx=%d", currentTaskIndex);
    lv_label_set_text(details_title_label, "No Task");
    lv_label_set_text(details_desc_label, "");
    return;
  }
  
  DIAG2("UI", "updateDetailsScreen idx=%d title=\"%s\"", currentTaskIndex, task->title.c_str());
  lv_label_set_text(details_title_label, task->title.c_str());
  lv_label_set_text(details_desc_label, task->description.c_str());
  
  // Reset scroll back to the top for the newly opened task
  lv_obj_scroll_to_y(details_content, 0, LV_ANIM_OFF);
}

static void scrollDescription(int direction) {
  // Scroll the content container by 12px per detent. LVGL clamps the
  // scroll position to the content bounds automatically, so we don't
  // need to track/limit the offset ourselves.
  // NOTE: forceScreenRefresh() is intentionally NOT called here — it
  // re-runs the layout pass and clamps the scroll position back to 0.
  // The normal loop() -> updateDisplay() -> lv_timer_handler() tick
  // will pick up the invalidation and redraw naturally.
  lv_obj_scroll_by(details_content, 0, -12 * direction, LV_ANIM_OFF);
  DIAG1("UI", "scrollDescription dir=%d scroll_y=%d", direction, lv_obj_get_scroll_y(details_content));
}

void initUI() {
  totalTasks = getTaskCount();
  DIAG1("UI", "initUI totalTasks=%d", totalTasks);
  
  initGeminiGradient();
  createHomeScreen();
  createDetailsScreen();
  
  lv_scr_load(screen_home);
  DIAG1("UI", "initUI: home screen loaded screen=%p", screen_home);
  
  updateHomeScreen();
  
  // Force immediate redraw
  lv_refr_now(NULL);
  DIAG1("UI", "initUI complete");
}

void updateUI() {
  DIAG2("UI", "updateUI state=%d", currentState);
  if (currentState == UI_HOME) {
    updateHomeScreen();
  } else if (currentState == UI_DETAILS) {
    updateDetailsScreen();
  }
}

void handleInputEvent(InputEvent event) {
  DIAG1("UI", "handleInputEvent event=%d state=%d idx=%d/%d animFlag=%d",
        event, currentState, currentTaskIndex, totalTasks, (int)animationInProgress);
  
  if (currentState == UI_HOME) {
    switch (event) {
      case ROTATE_NEXT:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          
          // Update task index with wrapping
          int prevIdx = currentTaskIndex;
          currentTaskIndex++;
          if (currentTaskIndex >= totalTasks) {
            currentTaskIndex = 0; // Wrap to first task
          }
          DIAG1("UI", "HOME ROTATE_NEXT: idx %d -> %d animFlag=%d",
                prevIdx, currentTaskIndex, (int)animationInProgress);
          
          // Only start animation if one isn't already running.
          // If an animation IS running, currentTaskIndex has already been updated
          // above. titleFadeOutReadyCb reads currentTaskIndex when it fires, so
          // the next content update will show the correct task automatically.
          // DO NOT touch LVGL labels here — the animation is actively modifying
          // them and calling lv_label_set_text mid-animation corrupts object state.
          if (!animationInProgress) {
            startTitleFadeAnimation();
          } else {
            DIAG1("UI", "Anim in progress - queued idx=%d (will show at fade-out ready)", currentTaskIndex);
          }
        } else {
          DIAG2("UI", "ROTATE_NEXT skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
        }
        break;
      case ROTATE_PREV:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          
          // Update task index with wrapping
          int prevIdxP = currentTaskIndex;
          currentTaskIndex--;
          if (currentTaskIndex < 0) {
            currentTaskIndex = totalTasks - 1; // Wrap to last task
          }
          DIAG1("UI", "HOME ROTATE_PREV: idx %d -> %d animFlag=%d",
                prevIdxP, currentTaskIndex, (int)animationInProgress);
          
          // Same safe pattern as ROTATE_NEXT above.
          if (!animationInProgress) {
            startTitleFadeAnimation();
          } else {
            DIAG1("UI", "Anim in progress - queued idx=%d (will show at fade-out ready)", currentTaskIndex);
          }
        } else {
          DIAG2("UI", "ROTATE_PREV skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
        }
        break;
      case CLICK:
        DIAG1("UI", "HOME CLICK -> details");
        setUIState(UI_DETAILS);
        break;
      case DOUBLE_CLICK:
        DIAG1("UI", "HOME DOUBLE_CLICK (no action)");
        break;
      case LONG_PRESS:
        DIAG1("UI", "HOME LONG_PRESS (no action)");
        break;
      default:
        DIAG1("UI", "HOME unknown event=%d", event);
        break;
    }
  } else if (currentState == UI_DETAILS) {
    switch (event) {
      case CLICK:
        DIAG1("UI", "DETAILS CLICK (no action)");
        break;
      case LONG_PRESS:
        DIAG1("UI", "DETAILS LONG_PRESS -> home");
        setUIState(UI_HOME);
        break;
      case ROTATE_NEXT:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          DIAG1("UI", "DETAILS ROTATE_NEXT -> scrollDown");
          scrollDescription(1);
        } else {
          DIAG2("UI", "ROTATE_NEXT skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
        }
        break;
      case ROTATE_PREV:
        rotationCount++;
        if (rotationCount >= ENCODER_RATIO) {
          rotationCount = 0;
          DIAG1("UI", "DETAILS ROTATE_PREV -> scrollUp");
          scrollDescription(-1);
        } else {
          DIAG2("UI", "ROTATE_PREV skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
        }
        break;
      default:
        DIAG1("UI", "DETAILS unknown event=%d", event);
        break;
    }
  }
}

void setUIState(UIState state) {
  DIAG1("UI", "setUIState %d -> %d", currentState, state);
  cancelAnimation(); // kill any in-flight animation before switching screens
  currentState = state;
  rotationCount = 0; // Reset encoder ratio counter on screen transition
  if (state == UI_HOME) {
    lv_scr_load(screen_home);
    DIAG1("UI", "Home screen loaded");
    updateHomeScreen();
    lv_refr_now(NULL); // Force immediate redraw
  } else if (state == UI_DETAILS) {
    lv_scr_load(screen_details);
    DIAG1("UI", "Details screen loaded");
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
    DIAG1("UI", "setCurrentTaskIndex %d -> %d", currentTaskIndex, index);
    currentTaskIndex = index;
  }
}

int getCurrentTaskIndex() {
  return currentTaskIndex;
}

void refreshTaskDisplay() {
  DIAG1("UI", "refreshTaskDisplay");
  updateUI();
  forceScreenRefresh();
}