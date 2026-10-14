#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// CONFIGURACIÓN DE PANTALLA Y DIMENSIONES
// ==========================================
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  320
#define SCREEN_ROTATION 0 // 0 = Vertical normal, 2 = Vertical invertido

// Pines TFT (HSPI)
#define TFT_MISO_PIN   12
#define TFT_MOSI_PIN   13
#define TFT_SCLK_PIN   14
#define TFT_CS_PIN     15
#define TFT_DC_PIN      2
#define TFT_RST_PIN    -1
#define TFT_BL_PIN     21

// Pines Táctil Resistivo XPT2046 (VSPI)
#define TOUCH_MOSI_PIN 32
#define TOUCH_MISO_PIN 39
#define TOUCH_CLK_PIN  25
#define TOUCH_CS_PIN   33
#define TOUCH_IRQ_PIN  36

// ==========================================
// PINES DE PERIFÉRICOS
// ==========================================
// LED RGB integrado (Lógica activa a nivel BAJO: 0 = Encendido, 1 = Apagado)
#define LED_RED_PIN    4
#define LED_GREEN_PIN  16
#define LED_BLUE_PIN   17

// Zumbador / Altavoz (PWM / DAC)
#define BUZZER_PIN     26
#define BUZZER_CHANNEL  0

// Sensor de luz ambiente (LDR)
#define LDR_PIN        34

// ==========================================
// PARÁMETROS DEL JUEGO (TAMAGOTCHI)
// ==========================================
#define TICK_INTERVAL_MS     5000   // Cada 5 segundos se evalúa el desgaste
#define SAVE_INTERVAL_MS    30000   // Guardado automático en NVS cada 30 segundos
#define MAX_POOPS               4   // Máximo de cacas acumuladas en pantalla
#define HUNGER_DECREASE_TICK    1   // Aumenta hambre por tick
#define HAPPINESS_DECREASE_TICK 1   // Baja felicidad por tick
#define ENERGY_DECREASE_TICK    1   // Baja energía por tick si está despierto

#endif // CONFIG_H
