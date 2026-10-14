#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

enum LedColor {
    LED_COLOR_OFF,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW,
    LED_COLOR_CYAN,
    LED_COLOR_MAGENTA,
    LED_COLOR_WHITE
};

void status_led_init();
void status_led_set(LedColor color);

#endif // STATUS_LED_H
