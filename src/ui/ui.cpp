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
static void applyHomeTaskContent();

// Returns true every ENCODER_RATIO-th call, implementing detent skipping.
static bool encoderTick() {
  rotationCount++;
  if (rotationCount >= ENCODER_RATIO) {
    rotationCount = 0;
    return true;
  }
  return false;
}

static void resetObjStyle(lv_obj_t* obj) {
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_style_radius(obj, 0, 0);
  lv_obj_set_style_pad_all(obj, 0, 0);
  lv_obj_set_style_shadow_width(obj, 0, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void startFadeAnim(void* var, int32_t from, int32_t to,
                          lv_anim_exec_xcb_t exec_cb,
                          lv_anim_ready_cb_t ready_cb,
                          lv_anim_path_cb_t path_cb) {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, var);
  lv_anim_set_values(&a, from, to);
  lv_anim_set_time(&a, 150);
  lv_anim_set_exec_cb(&a, exec_cb);
  lv_anim_set_ready_cb(&a, ready_cb);
  lv_anim_set_path_cb(&a, path_cb);
  lv_anim_start(&a);
}

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
  resetObjStyle(bar);
  lv_obj_set_style_bg_color(bar, COL_GRAD_A, 0);
  lv_obj_set_style_bg_grad(bar, &gemini_grad, 0);
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
  resetObjStyle(scr);
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
  resetObjStyle(home_priority_dot);
  lv_obj_set_style_radius(home_priority_dot, LV_RADIUS_CIRCLE, 0);
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

  applyHomeTaskContent();

  startFadeAnim(home_title_label, LV_OPA_0, LV_OPA_COVER,
                titleFadeInCallback, titleFadeInReadyCb, lv_anim_path_ease_out);
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

  startFadeAnim(home_title_label, LV_OPA_COVER, LV_OPA_0,
                titleFadeOutCallback, titleFadeOutReadyCb, lv_anim_path_ease_in);
  DIAG1("ANIM", "FadeOut started");
}

// Populate the home screen labels/dot from the current task.
// Safe to call from both animation callbacks and direct updates.
static void applyHomeTaskContent() {
  const Task* task = getTask(currentTaskIndex);
  if (!task) {
    DIAG1("UI", "applyHomeTaskContent: no task at idx=%d", currentTaskIndex);
    if (home_title_label)    lv_label_set_text(home_title_label, "No Tasks");
    if (home_priority_dot)   lv_obj_add_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
    if (home_position_label) lv_label_set_text(home_position_label, "");
    if (home_due_label)      lv_label_set_text(home_due_label, "Open App to Sync");
    return;
  }

  DIAG2("UI", "applyHomeTaskContent idx=%d title=\"%s\"", currentTaskIndex, task->title.c_str());
  if (home_priority_dot) {
    lv_obj_clear_flag(home_priority_dot, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(home_priority_dot, getPriorityColor(task->priority), 0);
  }

  char posStr[16];
  snprintf(posStr, sizeof(posStr), "%d / %d", currentTaskIndex + 1, totalTasks);
  if (home_position_label) lv_label_set_text(home_position_label, posStr);
  if (home_title_label)    lv_label_set_text(home_title_label, task->title.c_str());
  if (home_due_label)      lv_label_set_text(home_due_label, getDueDateString(task->dueDate));
}

static void updateHomeScreen() {
  // NOTE: do NOT call cancelAnimation() here. This function is called from
  // titleFadeOutReadyCb (mid-animation) and must not disturb the animation
  // lifecycle. Callers that need to cancel first (e.g. setUIState) do so
  // themselves before calling this.
  applyHomeTaskContent();
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
      case ROTATE_PREV: {
        if (!encoderTick()) {
          DIAG2("UI", "ROTATE skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
          break;
        }
        int prevIdx = currentTaskIndex;
        if (event == ROTATE_NEXT) {
          currentTaskIndex = (currentTaskIndex + 1) % totalTasks;
        } else {
          currentTaskIndex = (currentTaskIndex - 1 + totalTasks) % totalTasks;
        }
        DIAG1("UI", "HOME %s: idx %d -> %d animFlag=%d",
              event == ROTATE_NEXT ? "ROTATE_NEXT" : "ROTATE_PREV",
              prevIdx, currentTaskIndex, (int)animationInProgress);
        // Only start animation if one isn't already running.
        // titleFadeOutReadyCb reads currentTaskIndex when it fires, so
        // the next content update will show the correct task automatically.
        if (!animationInProgress) {
          startTitleFadeAnimation();
        } else {
          DIAG1("UI", "Anim in progress - queued idx=%d (will show at fade-out ready)", currentTaskIndex);
        }
        break;
      }
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
      case ROTATE_PREV:
        if (encoderTick()) {
          int dir = (event == ROTATE_NEXT) ? 1 : -1;
          DIAG1("UI", "DETAILS %s -> scroll", dir > 0 ? "ROTATE_NEXT" : "ROTATE_PREV");
          scrollDescription(dir);
        } else {
          DIAG2("UI", "ROTATE skipped (rotCount=%d/%d)", rotationCount, ENCODER_RATIO);
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
  } else if (state == UI_DETAILS) {
    lv_scr_load(screen_details);
    DIAG1("UI", "Details screen loaded");
    updateDetailsScreen();
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