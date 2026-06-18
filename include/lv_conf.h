#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* Enable complex drawing (needed for multi-stop gradients, rounded masks, etc.)
   and allow up to 3 gradient stops for the blue->purple->pink Gemini accent. */
#define LV_DRAW_COMPLEX 1
#define LV_GRADIENT_MAX_STOPS 3

#define LV_MEM_SIZE (64 * 1024U)
#define LV_MEM_ADR 0
#define LV_MEM_BUF_MAX_NUM 16

/* Tell LVGL to use millis() as its time source so animations actually advance */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE <Arduino.h>
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

/* Display refresh period (ms). Caps animation/scroll FPS at 1000/period.
   16ms => ~60 FPS for smooth scrolling and fades (feasible now that the
   SPI clock is high enough to push a full frame in ~9ms). */
#define LV_DEF_REFR_PERIOD 16

#define LV_INDEV_DEF_READ_PERIOD 30
#define LV_INDEV_SCROLL_THROW 10
#define LV_INDEV_DRAG_THROW 20

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_FONT_FMT_COMPRESSED 0
#define LV_FONT_SUBPX 0

#define LV_USE_ANIMATION 1
#define LV_ANIM_REPEAT_INFINITE 0
#define LV_ANIM_REPEAT_COUNT 0

#define LV_USE_LABEL 1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_LABEL_LONG_TXT_CTRL 1
#define LV_LABEL_WAIT_CHAR_COUNT 3
#define LV_LABEL_DOT_NUM 3
#define LV_LABEL_SCROLL_SPEED 25

#define LV_USE_IMG 1
#define LV_USE_LINE 1
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BUTTON 1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CALENDAR 0
#define LV_USE_CANVAS 0
#define LV_USE_CHART 0
#define LV_USE_CHECKBOX 0
#define LV_USE_DROPDOWN 0
#define LV_USE_IMAGEBUTTON 0
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 0
#define LV_USE_LIST 1
#define LV_USE_MENU 0
#define LV_USE_MSGBOX 0
#define LV_USE_ROLLER 0
#define LV_USE_SCALE 0
#define LV_USE_SLIDER 0
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 0
#define LV_USE_SWITCH 0
#define LV_USE_TEXTAREA 0
#define LV_USE_TABLE 0
#define LV_USE_TABVIEW 0
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_FONT_SMALL &lv_font_montserrat_12
#define LV_THEME_DEFAULT_FONT_NORMAL &lv_font_montserrat_14
#define LV_THEME_DEFAULT_FONT_LARGE &lv_font_montserrat_16

#define LV_USE_LOG 0
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 0

#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

#define LV_USE_FILESYSTEM 0
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

#define LV_USE_USER_DATA 1

#define LV_USE_GROUP 1
#define LV_GROUP_DEF_SET 1

#define LV_USE_GPU 0
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0

#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0

#define LV_USE_FREETYPE 0
#define LV_USE_FONT_COMPRESSED 0

#define LV_USE_FFMPEG 0

#define LV_USE_FREETYPE 0

#endif
