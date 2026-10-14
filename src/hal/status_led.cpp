#include "status_led.h"
#include "config.h"

void status_led_init() {
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    status_led_set(LED_COLOR_OFF);
}

void status_led_set(LedColor color) {
    // LED RGB en CYD es activo a nivel BAJO (LOW = encendido, HIGH = apagado)
    bool r = HIGH;
    bool g = HIGH;
    bool b = HIGH;

    switch (color) {
        case LED_COLOR_RED:
            r = LOW;
            break;
        case LED_COLOR_GREEN:
            g = LOW;
            break;
        case LED_COLOR_BLUE:
            b = LOW;
            break;
        case LED_COLOR_YELLOW:
            r = LOW; g = LOW;
            break;
        case LED_COLOR_CYAN:
            g = LOW; b = LOW;
            break;
        case LED_COLOR_MAGENTA:
            r = LOW; b = LOW;
            break;
        case LED_COLOR_WHITE:
            r = LOW; g = LOW; b = LOW;
            break;
        case LED_COLOR_OFF:
        default:
            r = HIGH; g = HIGH; b = HIGH;
            break;
    }

    digitalWrite(LED_RED_PIN, r);
    digitalWrite(LED_GREEN_PIN, g);
    digitalWrite(LED_BLUE_PIN, b);
}
