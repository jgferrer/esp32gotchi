# 👾 ESP32gotchi — 16-Bit Retro Virtual Pet

<div align="center">

![ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino-blue.svg)
![GUI](https://img.shields.io/badge/GUI-LVGL%20v8.3-orange.svg)
![Graphics Driver](https://img.shields.io/badge/Driver-LovyanGFX-purple.svg)
![IDE](https://img.shields.io/badge/IDE-PlatformIO-brightgreen.svg)
![Hardware](https://img.shields.io/badge/Board-ESP32--2432S028%20(CYD)-yellow.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)

**Una mascota virtual nostálgica estilo Tamagotchi con gráficos retro de 16 bits, pantalla táctil, efectos de sonido, gestión de Wi-Fi, sincronización horaria por NTP y persistencia en memoria Flash (NVS), desarrollada específicamente para la placa ESP32-2432S028 ("Cheap Yellow Display" / CYD).**

[Características](#-características-principales) •
[Hardware Requerido](#-hardware-requerido) •
[Instalación Rápida](#-instalación-y-compilación) •
[Cómo Jugar](#-mecánicas-y-uso) •
[Arquitectura](#-estructura-del-proyecto) •
[Documentación Hardware](#-especificaciones-y-desarrollo)

</div>

---

## ✨ Características Principales

* **🎮 3 Mascotas Seleccionables:**
  * 🦖 **Dino:** El clásico reptil prehistórico.
  * 🐥 **Chik:** El polluelo enérgico y cantarín.
  * 🐱 **Cat:** El felino curioso y juguetón.
* **🐣 Ciclo de Vida Completo y Evolución:**
  * Estados biológicos: *Huevo* ➔ *Bebé* ➔ *Adolescente* ➔ *Adulto*.
  * Mecánicas de supervivencia: hambre, aburrimiento, cansancio, suciedad, enfermedad y muerte (*si descuidas a tu mascota*).
  * Estados animados: durmiendo (luz apagada), comiendo, jugando, haciendo caca y celebrando.
* **🎨 Gráficos Retro Pixel-Art a Color:**
  * Sprites optimizados en color RGB565 de 16 bits con canal de transparencia (*Chroma Key*).
  * Renderizado acelerado por hardware mediante **LovyanGFX** y **LVGL 8.3**.
* **🔊 Efectos de Sonido Chiptune (8-bit):**
  * Melodías y bips retro a través del zumbador piezoeléctrico integrado (alimentar, limpiar, enfermedad, felicidad, clics).
* **💡 Indicador LED RGB:**
  * Muestra el estado físico de la mascota (verde = saludable, amarillo = atención/hambre, rojo = crítico/enfermo).
* **📶 Gestor de Wi-Fi y Reloj NTP en Pantalla:**
  * Menú de ajustes modal con teclado táctil completo QWERTY para conectar a redes Wi-Fi domésticas.
  * Escáner de redes Wi-Fi en tiempo real.
  * Sincronización horaria automática por Internet (NTP) con reloj digital visible en pantalla.
  * Control de brillo de la pantalla mediante barra deslizante táctil.
* **💾 Persistencia de Estado en Memoria (NVS):**
  * Guardado automático del estado de la mascota, edad, puntos de cuidado y credenciales de red mediante `Preferences` de ESP-IDF. Tu mascota continúa su vida aunque desconectes la placa.

---

## 🛠 Hardware Requerido

Este proyecto está optimizado y configurado de fábrica para la placa **ESP32-2432S028** (conocida comúnmente como **CYD** o *"Cheap Yellow Display"*):

| Periférico | Componente en Placa | Detalles de Conexión |
| :--- | :--- | :--- |
| **Microcontrolador** | ESP32 Dual Core @ 240 MHz | 520 KB RAM, 4 MB Flash |
| **Pantalla** | TFT LCD 2.8" (240x320 px) | Controlador ST7789 / ILI9341 vía bus HSPI |
| **Panel Táctil** | Táctil resistivo XPT2046 | Configurado vía Software SPI para evitar conflictos |
| **Audio** | Zumbador / Altavoz piezoeléctrico | GPIO 26 (LEDC PWM) |
| **LED RGB** | LED de 3 colores | GPIOs 4 (R), 16 (G), 17 (B) — Lógica Active-LOW |
| **Sensor de Luz** | Fotorresistencia (LDR) | GPIO 34 (ADC1) |

> 📖 **Para una descripción técnica exhaustiva de los registros, buses, mapa de pines y calibraciones:** Consulta el archivo [**`ESPECIFICACIONES_DESARROLLO.md`**](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/ESPECIFICACIONES_DESARROLLO.md).

---

## 🚀 Instalación y Compilación

El proyecto utiliza **PlatformIO**, lo que permite compilar y flashear el firmware sin configurar librerías manualmente en el IDE de Arduino.

### Requisitos Previos
1. Instalar [Visual Studio Code](https://code.visualstudio.com/).
2. Instalar la extensión oficial de **PlatformIO IDE** en VS Code.
3. Conectar la placa ESP32 a la computadora mediante cable Micro-USB o USB-C de datos.

### Pasos para Flashear

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com/tu-usuario/esp32gotchi.git
   cd esp32gotchi
   ```

2. **Abrir en PlatformIO:**
   * Abre la carpeta `esp32gotchi` en Visual Studio Code.
   * PlatformIO descargará e instalará automáticamente las dependencias declaradas en `platformio.ini` (`LovyanGFX` y `LVGL`).

3. **Compilar y Subir el Firmware:**
   * Haz clic en el botón de **Upload** (icono de flecha `→`) en la barra inferior de PlatformIO, o ejecuta en la terminal integrada:
   ```bash
   pio run -t upload
   ```

4. **Monitor Serie (Opcional):**
   * Para depuración, abre la consola serie a 115200 baudios:
   ```bash
   pio device monitor
   ```

---

## 🕹 Mecánicas y Uso

### Pantalla de Inicio (Selección de Mascota)
Al encender la placa por primera vez (o tras revivir a una mascota fallecida), aparecerá la pantalla de selección donde podrás pulsar sobre el **Dino**, **Chik** o **Cat** para comenzar la crianza desde su estado de huevo.

### Botones de Acción (Barra Inferior)
* 🍎 **Comer:** Alimenta a tu mascota para reducir su nivel de hambre.
* 🧹 **Limpiar:** Retira las cacas acumuladas en pantalla para restablecer la higiene y prevenir enfermedades.
* ⚽ **Jugar:** Incrementa la felicidad y el vínculo afectivo a cambio de un ligero consumo de energía.
* 💤 **Dormir:** Apaga la luz de la habitación para que la mascota descanse y recupere su barra de energía.
* 💊 **Curar:** Administra medicinas cuando la mascota enferme debido a falta de higiene o descuido.

### Menú de Ajustes (Icono de Engranaje ⚙️)
Pulsando sobre el botón superior izquierdo accederás al panel de ajustes:
* **Slider de Brillo:** Permite atenuar la pantalla para ahorrar batería o no deslumbrar en la noche.
* **Conexión Wi-Fi:** Activa el switch de Wi-Fi, pulsa en *Buscar* para escanear las redes del entorno, selecciona tu SSID e ingresa la contraseña mediante el teclado virtual en pantalla.
* **Sincronización NTP:** En cuanto se establece la conexión Wi-Fi, el reloj de la esquina superior derecha se sincroniza automáticamente con la hora real de Internet.

---

## 📂 Estructura del Proyecto

El código está estructurado siguiendo principios de código limpio y separación de capas:

```
esp32gotchi/
├── include/
│   ├── config.h             # Pines de hardware, tiempos de ciclo y ajustes del juego
│   └── lv_conf.h            # Configuración de rendimiento y módulos de LVGL 8.3
├── src/
│   ├── hal/                 # Capa de Abstracción de Hardware (HAL)
│   │   ├── display_driver.* # Integración LovyanGFX y callbacks de refresco/táctil LVGL
│   │   ├── status_led.*     # Control del LED RGB integrado (Active-LOW)
│   │   ├── buzzer.*         # Efectos de audio chiptune retro vía PWM
│   │   └── wifi_manager.*   # Escáner Wi-Fi, reconexión automática y sincronización NTP
│   ├── ui/                  # Capa de Presentación (UI)
│   │   ├── ui.*             # Construcción de pantallas, barras y widgets en LVGL
│   │   └── game/pet_sprites.h # Tablas de píxeles binarios de los sprites de 16 bits
│   ├── game/                # Lógica del Juego (Motor de la Mascota)
│   │   ├── tamagotchi.*     # Máquina de estados, evolución, salud y desgaste
│   │   └── pet_storage.*    # Guardado y recuperación persistente en memoria NVS (Flash)
│   └── main.cpp             # Bucle principal de ejecución y despacho de tareas cooperativas
├── platformio.ini           # Configuración del entorno de compilación, flags y librerías
├── ESPECIFICACIONES_DESARROLLO.md # Guía técnica de la placa y manual para nuevos desarrollos
└── README.md                # Portada principal del repositorio
```

---

## 📑 Especificaciones y Desarrollo

¿Quieres usar esta misma placa y código base para desarrollar **otras aplicaciones** (como dashboards de domótica, consolas portátiles o monitores industriales)?

Revisa el archivo [**`ESPECIFICACIONES_DESARROLLO.md`**](file:///C:/Users/jgferrer/Downloads/esp32/esp32gotchi/ESPECIFICACIONES_DESARROLLO.md), donde encontrarás:
* Diagrama completo de pines y buses (HSPI, VSPI, puertos de expansión JST).
* Boilerplate mínimo para iniciar un proyecto nuevo desde cero.
* Solución a conflictos comunes de bus SPI entre pantalla y panel táctil.
* Guía de uso del sensor de luz analógico LDR en GPIO 34.

---

## 📄 Licencia

Este proyecto se distribuye bajo la licencia **MIT**. Puedes usarlo, modificarlo y distribuirlo libremente para proyectos personales o educativos.
