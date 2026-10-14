#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "config.h"

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void);
};

extern LGFX tft;

void display_driver_init();

#endif // DISPLAY_DRIVER_H
