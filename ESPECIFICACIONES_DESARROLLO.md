# Especificaciones Técnicas y Guía de Desarrollo: ESP32-2432S028 (CYD)

Este documento detalla todas las especificaciones de hardware, asignación de pines, configuración del entorno de desarrollo, librerías y patrones de arquitectura empleados en este proyecto. Su objetivo es servir como **manual y plantilla técnica de referencia** para crear nuevas aplicaciones (paneles de control domótico, dashboards IoT, juegos retro, reproductores multimedia, monitores de sensores, etc.) sobre esta misma placa.

---

## 1. Identificación del Hardware

La placa base utilizada en este proyecto es la popular **ESP32-2432S028** (conocida en la comunidad maker como **CYD** o *"Cheap Yellow Display"*).

### Ficha Técnica de la Placa
| Componente | Especificación |
| :--- | :--- |
| **Microcontrolador** | ESP32-D0WD-V3 / ESP32-WROOM-32 (Xtensa® Dual-Core 32-bit LX6 @ hasta 240 MHz) |
| **Memoria RAM** | 520 KB SRAM interna |
| **Memoria Flash** | 4 MB SPI Flash externa |
| **Pantalla Integrada** | TFT LCD 2.8 pulgadas a color (resolución 240 × 320 px QVGA, color RGB565 de 16 bits) |
| **Controlador de Pantalla** | **ST7789** (o compatible **ILI9341** según lote de fabricación), interfaz HSPI |
| **Panel Táctil** | Resistivo de 4 hilos con controlador **XPT2046**, interfaz SPI por software |
| **Audio** | Zumbador / Altavoz pasivo integrado o salida de audio mono en conector JST |
| **Indicadores** | LED RGB discreto de alto brillo integrado en la placa |
| **Sensores** | Fotorresistencia (LDR / CDS) analógica para detección de luz ambiente |
| **Almacenamiento Externo** | Ranura MicroSD integrada (bus SPI independiente en pines VSPI) |
| **Conectividad Inalámbrica** | Wi-Fi 802.11 b/g/n (2.4 GHz) + Bluetooth 4.2 BR/EDR & BLE |
| **Alimentación y Debug** | Puerto Micro-USB / USB-C para programación (CH340 / CP2102) y alimentación (5V DC) |

---

## 2. Mapa Completo de Pines (Pinout)

Para evitar colisiones entre buses y pines compartidos, a continuación se detalla la asignación de pines probada y funcional en este proyecto:

### 2.1. Pantalla TFT (Bus HSPI)
| Pin ESP32 | Nombre / Señal | Función | Observaciones |
| :---: | :---: | :--- | :--- |
| **GPIO 14** | `TFT_SCLK` | Reloj SPI (Clock) | Frecuencia de hasta 40 MHz |
| **GPIO 13** | `TFT_MOSI` | Datos SPI (Master Out) | Envío de comandos y píxeles |
| **GPIO 12** | `TFT_MISO` | Datos SPI (Master In) | Generalmente no leído por el driver |
| **GPIO 15** | `TFT_CS` | Chip Select Pantalla | Activo a nivel bajo |
| **GPIO 2** | `TFT_DC` | Data / Command | Selección de comando (LOW) o dato (HIGH) |
| **GPIO -1** | `TFT_RST` | Reset Pantalla | Conectado a la línea de reset general (EN) |
| **GPIO 21** | `TFT_BL` | Retroiluminación (Backlight) | Control por modulación PWM (brillo regulable) |

### 2.2. Panel Táctil Resistivo XPT2046 (Bus SPI Software)
> [!IMPORTANT]
> En la placa CYD, los pines del panel táctil están conectados a GPIOs independientes que **no coinciden** con un bus hardware SPI nativo completo. La mejor solución —y la utilizada en este proyecto— es configurar el controlador táctil mediante **Software SPI** (`cfg.spi_host = -1` en LovyanGFX), garantizando máxima estabilidad sin colisionar con la pantalla.

| Pin ESP32 | Nombre / Señal | Función | Observaciones |
| :---: | :---: | :--- | :--- |
| **GPIO 25** | `TOUCH_CLK` | Reloj Táctil (Clock) | Frecuencia recomendada: 1.0 MHz |
| **GPIO 32** | `TOUCH_MOSI` | Entrada de datos táctil | Datos de configuración al XPT2046 |
| **GPIO 39** | `TOUCH_MISO` | Salida de datos táctil | Solo entrada (`SENSOR_VN`) |
| **GPIO 33** | `TOUCH_CS` | Chip Select Táctil | Activo a nivel bajo |
| **GPIO 36** | `TOUCH_IRQ` | Interrupción de toque | Opcional (se puede consultar por sondeo / polling) |

### 2.3. Periféricos Integrados
| Pin ESP32 | Periférico | Tipo | Particularidad / Lógica |
| :---: | :---: | :---: | :--- |
| **GPIO 4** | **LED Rojo** | Salida digital / PWM | **Activo a nivel BAJO (`LOW` = Encendido)** |
| **GPIO 16** | **LED Verde** | Salida digital / PWM | **Activo a nivel BAJO (`LOW` = Encendido)** |
| **GPIO 17** | **LED Azul** | Salida digital / PWM | **Activo a nivel BAJO (`LOW` = Encendido)** |
| **GPIO 26** | **Buzzer / Audio** | Salida PWM / DAC | Conectado al canal LEDC 0 (o DAC Canal 2 de 8 bits) |
| **GPIO 34** | **Sensor LDR** | Entrada analógica | Conectado a ADC1_CH6 (seguro de usar con Wi-Fi) |
| **GPIO 0** | **Botón BOOT** | Entrada digital | Pulsador integrado para modo bootloader o uso general |

### 2.4. Ranura MicroSD (Bus VSPI)
Si tu nueva aplicación necesita leer imágenes, fuentes, archivos de audio o registros en tarjeta MicroSD:
| Pin ESP32 | Señal MicroSD | Observaciones |
| :---: | :---: | :--- |
| **GPIO 18** | `SD_SCK` | Bus SPI nativo (VSPI CLK) |
| **GPIO 19** | `SD_MISO` | Bus SPI nativo (VSPI MISO) |
| **GPIO 23** | `SD_MOSI` | Bus SPI nativo (VSPI MOSI) |
| **GPIO 5** | `SD_CS` | Chip Select de la tarjeta SD |

### 2.5. Puertos de Expansión Externos (Conectores JST)
La placa incluye conectores JST de paso fino (1.25 mm) en el reverso para conectar sensores I2C, UART u otros periféricos:
- **Conector CN1 / P3:**
  - `GPIO 35`: Entrada analógica/digital pura (sin pull-up interno).
  - `GPIO 22`: E/S digital, compatible con I2C SCL.
  - `GPIO 27`: E/S digital, compatible con I2C SDA.
  - `GND` y `3.3V` / `VIN`.

---

## 3. Configuración del Entorno de Desarrollo (PlatformIO)

El archivo [`platformio.ini`](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/platformio.ini) configurado para exprimir el rendimiento de la placa:

```ini
[env:esp32-2432s028]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

build_flags = 
    -D CORE_DEBUG_LEVEL=1          ; Nivel de logs de ESP-IDF (0=None, 1=Error, 2=Warn, 3=Info)
    -D LV_CONF_INCLUDE_SIMPLE       ; Permite incluir #include <lv_conf.h> directamente
    -I include                      ; Incluye la carpeta include en la ruta de cabeceras

lib_deps =
    lovyan03/LovyanGFX @ ^1.1.16    ; Driver gráfico acelerado por hardware para pantalla y táctil
    lvgl/lvgl @ ^8.3.11             ; Motor de interfaz gráfica (widgets, temas, eventos táctiles)
```

> [!TIP]
> **Subida a 921600 baudios:** La mayoría de conversores USB-Serie de estas placas soportan `upload_speed = 921600`. Si experimentas errores intermitentes al subir el código, puedes reducir temporalmente a `460800` o `115200`.

---

## 4. Patrón de Arquitectura del Software

El proyecto sigue una arquitectura modular en 4 capas para aislar el hardware de la lógica de negocio y de la presentación:

```
esp32gotchi/
├── include/
│   ├── config.h             <-- Definición centralizada de pines y constantes globales
│   └── lv_conf.h            <-- Configuración a medida de LVGL (memoria, fuentes, widgets)
├── src/
│   ├── hal/                 <-- Capa de Abstracción de Hardware (HAL)
│   │   ├── display_driver.h/.cpp <-- LovyanGFX + Binding de pantalla y táctil con LVGL
│   │   ├── status_led.h/.cpp     <-- Control de colores del LED RGB (Active-LOW)
│   │   ├── buzzer.h/.cpp         <-- Generación de tonos y melodías PWM
│   │   └── wifi_manager.h/.cpp   <-- Gestión de Wi-Fi, escaneo, reconexión y sincronización NTP
│   ├── ui/                  <-- Capa de Presentación (LVGL)
│   │   ├── ui.h/.cpp             <-- Creación de pantallas, widgets y refresco visual
│   │   └── assets / sprites      <-- Recursos gráficos binarios en memoria Flash
│   ├── game/ (o domain/)    <-- Lógica pura de la aplicación
│   │   ├── tamagotchi.h/.cpp     <-- Máquina de estados o modelo de datos
│   │   └── pet_storage.h/.cpp    <-- Persistencia en memoria no volátil (NVS/Preferences)
│   └── main.cpp             <-- Orquestador principal (setup() y loop() cooperativo)
```

---

## 5. Guías de Implementación por Módulo

### 5.1. Pantalla y Táctil con LovyanGFX y LVGL 8

La combinación de **LovyanGFX** + **LVGL v8.3** proporciona una interfaz fluida con animaciones y widgets táctiles a 30-60 FPS gracias a las transferencias DMA.

#### Clase de Dispositivo LovyanGFX personalizada ([`display_driver.cpp`](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/src/hal/display_driver.cpp))
```cpp
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  _panel_instance; // O lgfx::Panel_ILI9341 según tu pantalla
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void) {
        // 1. Bus SPI para Pantalla (HSPI)
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = HSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read  = 16000000;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk   = TFT_SCLK_PIN; // 14
            cfg.pin_mosi   = TFT_MOSI_PIN; // 13
            cfg.pin_miso   = TFT_MISO_PIN; // 12
            cfg.pin_dc     = TFT_DC_PIN;   // 2
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // 2. Panel TFT
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = TFT_CS_PIN; // 15
            cfg.pin_rst          = TFT_RST_PIN; // -1
            cfg.panel_width      = 240;
            cfg.panel_height     = 320;
            cfg.offset_rotation  = 0;          // 0 = Vertical, 1 = Horizontal
            cfg.invert           = false;      // Ajustar si los colores se ven invertidos
            cfg.rgb_order        = false;
            _panel_instance.config(cfg);
        }

        // 3. Retroiluminación (PWM)
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = TFT_BL_PIN; // 21
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // 4. Panel Táctil Resistivo (Software SPI)
        {
            auto cfg = _touch_instance.config();
            cfg.pin_cs     = TOUCH_CS_PIN;   // 33
            cfg.pin_sclk   = TOUCH_CLK_PIN;  // 25
            cfg.pin_mosi   = TOUCH_MOSI_PIN; // 32
            cfg.pin_miso   = TOUCH_MISO_PIN; // 39
            cfg.spi_host   = -1;             // -1 = Software SPI (vital en CYD)
            cfg.freq       = 1000000;
            cfg.x_min      = 300;
            cfg.x_max      = 3900;
            cfg.y_min      = 3700;
            cfg.y_max      = 200;
            cfg.offset_rotation = 2;         // Calibración de orientación del toque
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

extern LGFX tft;
```

#### Integración y Buffers con LVGL
Para maximizar el rendimiento sin agotar la RAM interna del ESP32, se utiliza un buffer parcial de 40 líneas de pantalla:
- Dimensiones del buffer: `240 px × 40 líneas × 2 bytes/px = 19.2 KB RAM`.
- Se registran dos callbacks principales:
  1. `my_disp_flush`: Vuelca los píxeles renderizados por LVGL a la pantalla mediante `tft.writePixels()`.
  2. `my_touchpad_read`: Lee las coordenadas `(X, Y)` mediante `tft.getTouch(&touchX, &touchY)`.

---

### 5.2. LED RGB Integrado

El LED RGB montado en la placa funciona con **lógica invertida (Active-LOW)**:
- Escribir `LOW` (0 V) **enciende** el canal.
- Escribir `HIGH` (3.3 V) **apaga** el canal.

```cpp
// Inicialización
pinMode(LED_RED_PIN, OUTPUT);   // GPIO 4
pinMode(LED_GREEN_PIN, OUTPUT); // GPIO 16
pinMode(LED_BLUE_PIN, OUTPUT);  // GPIO 17

// Ejemplo para encender color Cyan (Verde + Azul)
digitalWrite(LED_RED_PIN, HIGH);  // Apagado
digitalWrite(LED_GREEN_PIN, LOW); // Encendido
digitalWrite(LED_BLUE_PIN, LOW);  // Encendido
```

> [!TIP]
> Si deseas graduación suave de colores en lugar de estados ON/OFF binarios, puedes conectar los 3 pines a canales PWM independientes con `ledcSetup(canal, 5000, 8)` y `ledcWrite(canal, 255 - valor)`.

---

### 5.3. Zumbador / Audio (GPIO 26)

El zumbador pasivo de la placa está conectado al **GPIO 26**. Se controla cómodamente mediante el periférico LEDC de hardware del ESP32:

```cpp
#define BUZZER_PIN 26
#define BUZZER_CHANNEL 0

void buzzer_init() {
    ledcSetup(BUZZER_CHANNEL, 2000, 8); // 2 kHz base, resolución 8 bits
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0);       // Silencio
}

void buzzer_beep(uint32_t freq, uint32_t duration_ms) {
    if (freq > 0) {
        ledcWriteTone(BUZZER_CHANNEL, freq);
    }
    delay(duration_ms);
    ledcWrite(BUZZER_CHANNEL, 0);
}
```

> [!NOTE]
> El **GPIO 26** también es la salida analógica **DAC 2** (`DAC_CHANNEL_2`) del ESP32. Si tu aplicación requiere reproducir clips de voz o audio PCM/WAV de 8 bits reales (sin PWM), puedes usar `dacWrite(26, sample)` o la librería `ESP32-audioI2S` con salida por DAC.

---

### 5.4. Sensor de Luz Ambiente (LDR en GPIO 34)

La fotorresistencia está conectada a un divisor resistivo con salida en el **GPIO 34**:
- Pin de solo entrada analógica (**ADC1 Canal 6**).
- **Ventaja clave:** Al pertenecer al bloque **ADC1**, puede leerse en cualquier momento sin interferir ni bloquearse con las transmisiones de la radio Wi-Fi (a diferencia de los pines ADC2).

```cpp
uint16_t read_ambient_light() {
    // Retorna valores entre 0 (oscuridad) y 4095 (luz directa brillante)
    return analogRead(34);
}

// Ejemplo: Ajuste dinámico de brillo de pantalla según la luz del entorno
void auto_adjust_brightness() {
    uint16_t light = analogRead(34);
    // Mapear de 0-4095 a rango de brillo 30-255
    uint8_t target_brightness = map(light, 200, 3800, 40, 255);
    tft.setBrightness(target_brightness);
}
```

---

### 5.5. Conectividad Wi-Fi y Sincronización Horaria (NTP)

El módulo [`wifi_manager.cpp`](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/src/hal/wifi_manager.cpp) implementa un flujo desacoplado y no bloqueante para conexión Wi-Fi y sincronización NTP:

1. **Configuración de la zona horaria europea/española con cambio de hora automático (CEST/CET):**
   ```cpp
   // UTC+1 hora estándar (3600 s), UTC+2 en horario de verano (3600 s offset adicional)
   configTime(3600, 3600, "pool.ntp.org", "time.google.com");
   ```
2. **Escaneo de redes disponibles:**
   ```cpp
   WiFi.mode(WIFI_STA);
   WiFi.disconnect();
   int n = WiFi.scanNetworks();
   for (int i = 0; i < n; i++) {
       Serial.println(WiFi.SSID(i));
   }
   ```
3. **Comprobación periódica y reconexión en el loop:**
   Realizar comprobaciones cada 10-15 segundos sin usar bucles bloqueantes `while (status != WL_CONNECTED)`.

---

### 5.6. Persistencia de Datos (NVS / Preferences)

Para almacenar credenciales Wi-Fi, ajustes de usuario o estados de tu aplicación que deben persistir tras reiniciar o apagar la placa:
- Usar la librería oficial `<Preferences.h>` de ESP32 (utiliza el sistema de archivos NVS en Flash).

```cpp
#include <Preferences.h>

Preferences prefs;

void guardar_configuracion(const char* ssid, const char* pass, uint8_t brillo) {
    if (prefs.begin("miapp", false)) { // namespace de máx 15 caracteres
        prefs.putString("wifi_ssid", ssid);
        prefs.putString("wifi_pass", pass);
        prefs.putUChar("brillo", brillo);
        prefs.end();
    }
}

void cargar_configuracion() {
    if (prefs.begin("miapp", true)) { // Modo solo lectura
        String ssid = prefs.getString("wifi_ssid", "DefaultSSID");
        uint8_t brillo = prefs.getUChar("brillo", 200);
        prefs.end();
    }
}
```

---

## 6. Plantilla Base (Boilerplate) para Nuevos Proyectos

Si deseas crear una nueva aplicación partiendo de cero utilizando la arquitectura y los módulos HAL de esta carpeta:

### Archivo `src/main.cpp`
```cpp
#include <Arduino.h>
#include "config.h"
#include "hal/display_driver.h"
#include "hal/status_led.h"
#include "hal/buzzer.h"
#include "hal/wifi_manager.h"
#include <lvgl.h>

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[SISTEMA] Iniciando nueva aplicación...");

    // 1. Inicializar periféricos físicos
    status_led_init();
    status_led_set(LED_COLOR_BLUE);

    buzzer_init();
    buzzer_beep(1500, 100);

    // 2. Inicializar subsistema gráfico (LovyanGFX + LVGL)
    display_driver_init();

    // 3. Inicializar conectividad Wi-Fi y persistencia
    wifi_manager_init();

    // 4. Crear interfaz gráfica en LVGL
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "¡Hola ESP32 CYD!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    status_led_set(LED_COLOR_GREEN);
    Serial.println("[SISTEMA] Aplicación lista y corriendo.");
}

void loop() {
    // 1. Tareas de renderizado y eventos de LVGL
    lv_timer_handler();

    // 2. Actualización de red en segundo plano
    wifi_manager_update();

    // 3. Pequeño descanso para alimentar el watchdog y ceder CPU a FreeRTOS
    delay(5);
}
```

---

## 7. Puntos Críticos y Buenas Prácticas (Troubleshooting)

| Problema Típico | Causa Común | Solución Comprobada |
| :--- | :--- | :--- |
| **Pantalla en blanco o no enciende** | El pin de retroiluminación (GPIO 21) está en LOW o sin configurar el canal PWM. | Asegurar que `tft.setBrightness(255)` se llama después de `tft.init()`. |
| **Colores invertidos (negativo fotográfico)** | Diferencias de fabricación del panel IPS/TFT (ST7789 vs ILI9341). | Cambiar `cfg.invert = false;` a `true` (o viceversa) en la configuración del panel en [`display_driver.cpp`](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/src/hal/display_driver.cpp). |
| **El táctil no responde o las coordenadas están desfasadas** | Mala orientación de coordenadas o conflicto con el bus SPI de pantalla. | 1. Usar Software SPI (`cfg.spi_host = -1`).<br>2. Ajustar `cfg.offset_rotation` (generalmente 2 para rotación 0). |
| **Reinicio repentino (Watchdog Timer Panic)** | El bucle `loop()` contiene retardos bloqueantes largos (`delay(1000)` continuos) impidiendo que `lv_timer_handler()` procese tareas. | Usar temporizadores basados en `millis()` y llamar siempre a `lv_timer_handler()` con `delay(5)`. |
| **Fallo al leer sensores analógicos cuando el Wi-Fi está encendido** | El sensor está conectado a un pin del bloque **ADC2** (bloqueado por el hardware Wi-Fi de Espressif). | Usar únicamente pines del bloque **ADC1** (en esta placa el sensor LDR está en GPIO 34, que pertenece a ADC1 y es completamente seguro). |
| **El LED RGB muestra colores invertidos o apagados** | Los pines del LED RGB en el CYD son **Active-LOW**. | Poner el GPIO a `LOW` para encender el color y a `HIGH` para apagarlo. |
