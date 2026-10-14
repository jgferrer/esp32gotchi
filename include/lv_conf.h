/**
 * @file lv_conf.h
 * Configuración de LVGL v8.3 para ESP32-2432S028
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_CHROMA_KEY lv_color_hex(0xFF00FF)

#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE (48U * 1024U)
    #define LV_MEM_BUF_MAX_NUM 16
    #define LV_MEMCPY_MEMSET_STD 0
#endif

/* Utilizar millis() de Arduino para el tick de LVGL */
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/* Configuración de rendimiento */
#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 30
#define LV_DPI_DEF 130

/* Activación de fuentes estándar */
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Animaciones y temas */
#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW 1
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/* Widgets */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 1

/* Extra widgets */
#define LV_USE_KEYBOARD 1
#define LV_USE_ANIMIMG 1
#define LV_USE_LED 1
#define LV_USE_LIST 1
#define LV_USE_MENU 1
#define LV_USE_METER 1
#define LV_USE_MSGBOX 1
#define LV_USE_SPAN 1
#define LV_USE_SPINBOX 1
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 1
#define LV_USE_TILEVIEW 1
#define LV_USE_WIN 1

/* Log */
#define LV_USE_LOG 0

#endif /* LV_CONF_H */
