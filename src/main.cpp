#include <Arduino.h>
#include "config.h"
#include "hal/display_driver.h"
#include "hal/buzzer.h"
#include "hal/status_led.h"
#include "hal/wifi_manager.h"
#include "game/tamagotchi.h"
#include "ui/ui.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("=========================================");
    Serial.println("  ESP32 TAMAGOTCHI 16-BIT RETRO EDITION  ");
    Serial.println("=========================================");

    // 1. Inicialización de periféricos
    status_led_init();
    status_led_set(LED_COLOR_YELLOW);

    buzzer_init();
    buzzer_play_happy();

    // 2. Inicialización de pantalla TFT, táctil y LVGL
    Serial.println("[HAL] Inicializando pantalla y táctil...");
    display_driver_init();

    // 3. Inicialización del gestor de WiFi, reloj NTP y brillo
    Serial.println("[HAL] Inicializando WiFi, brillo y NTP...");
    wifi_manager_init();

    // 4. Inicialización del motor del juego (carga NVS)
    Serial.println("[GAME] Cargando mascota desde memoria...");
    pet.init();

    // 5. Inicialización de la interfaz táctil
    Serial.println("[UI] Creando interfaz de usuario...");
    ui_init();

    status_led_set(LED_COLOR_GREEN);
    Serial.println("[SYSTEM] ¡Sistema listo!");
}

void loop() {
    // 1. Manejador de eventos y renderizado de LVGL
    lv_timer_handler();

    // 2. Actualización de la lógica del Tamagotchi
    pet.update();

    // 3. Actualización de estado de red y reconexión
    wifi_manager_update();

    // 4. Refresco de barras, reloj y animaciones de la UI
    ui_update();

    // Pequeño retardo para no saturar la CPU
    delay(5);
}
