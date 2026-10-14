#include "display_driver.h"

LGFX::LGFX(void) {
    {
        auto cfg = _bus_instance.config();
        cfg.spi_host = HSPI_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read  = 16000000;
        cfg.spi_3wire  = false;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk   = TFT_SCLK_PIN;
        cfg.pin_mosi   = TFT_MOSI_PIN;
        cfg.pin_miso   = TFT_MISO_PIN;
        cfg.pin_dc     = TFT_DC_PIN;
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }

    {
        auto cfg = _panel_instance.config();
        cfg.pin_cs           = TFT_CS_PIN;
        cfg.pin_rst          = TFT_RST_PIN;
        cfg.pin_busy         = -1;
        cfg.panel_width      = SCREEN_WIDTH;
        cfg.panel_height     = SCREEN_HEIGHT;
        cfg.offset_x         = 0;
        cfg.offset_y         = 0;
        cfg.offset_rotation  = SCREEN_ROTATION;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits  = 1;
        cfg.readable         = false;
        cfg.invert           = false; // Inversión desactivada: restaura colores naturales
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = false;
        _panel_instance.config(cfg);
    }

    {
        auto cfg = _light_instance.config();
        cfg.pin_bl      = TFT_BL_PIN;
        cfg.invert      = false;
        cfg.freq        = 44100;
        cfg.pwm_channel = 7;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    {
        auto cfg = _touch_instance.config();
        cfg.pin_cs     = TOUCH_CS_PIN;
        cfg.pin_sclk   = TOUCH_CLK_PIN;
        cfg.pin_mosi   = TOUCH_MOSI_PIN;
        cfg.pin_miso   = TOUCH_MISO_PIN;
        cfg.pin_int    = -1;
        cfg.spi_host   = -1; // -1: Software SPI para evitar conflictos en CYD
        cfg.bus_shared = false;
        cfg.freq       = 1000000;
        cfg.x_min      = 300;
        cfg.x_max      = 3900;
        cfg.y_min      = 3700;
        cfg.y_max      = 200;
        cfg.offset_rotation = 2;
        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
}

LGFX tft;

// Buffer de dibujado de LVGL (40 líneas de altura = 240 * 40 * 2 bytes = 19.2 KB de RAM)
#define LV_BUF_LINES 40
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[SCREEN_WIDTH * LV_BUF_LINES];

/* Callback de refresco de pantalla para LVGL */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Callback de lectura de pulsación táctil para LVGL */
static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void display_driver_init() {
    tft.init();
    tft.setRotation(SCREEN_ROTATION); // Vertical (240x320)
    tft.setBrightness(255);
    Serial.printf("[HAL] Display inicializado (%s): %dx%d px\n", "ST7789", tft.width(), tft.height());

    lv_init();

    // Inicializar buffer de render
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * LV_BUF_LINES);

    // Inicializar driver de pantalla
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Inicializar driver táctil
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}
