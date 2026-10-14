#include "wifi_manager.h"
#include "display_driver.h"
#include <Preferences.h>
#include <time.h>

static Preferences cfg_prefs;
static const char* CFG_NAMESPACE = "tamacfg";

static WifiSettings current_settings = { false, "", "", 220 };

static bool ntp_configured = false;
static uint32_t last_wifi_check = 0;

void wifi_manager_init() {
    if (cfg_prefs.begin(CFG_NAMESPACE, false)) {
        current_settings.enabled = cfg_prefs.getBool("wifi_en", false);
        current_settings.brightness = cfg_prefs.getUChar("bright", 220);
        String s = cfg_prefs.getString("ssid", "");
        String p = cfg_prefs.getString("pass", "");
        strncpy(current_settings.ssid, s.c_str(), sizeof(current_settings.ssid) - 1);
        strncpy(current_settings.password, p.c_str(), sizeof(current_settings.password) - 1);
        cfg_prefs.end();
    }

    // Aplicar brillo guardado
    tft.setBrightness(current_settings.brightness);

    // Configurar zona horaria NTP (CET/CEST - Madrid / Europa Central: UTC+1 y +1h en horario de verano)
    configTime(3600, 3600, "pool.ntp.org", "time.google.com");
    ntp_configured = true;

    if (current_settings.enabled && strlen(current_settings.ssid) > 0) {
        wifi_manager_connect(current_settings.ssid, current_settings.password);
    } else {
        WiFi.mode(WIFI_OFF);
    }
}

void wifi_manager_save_settings(const WifiSettings& settings) {
    current_settings = settings;
    if (cfg_prefs.begin(CFG_NAMESPACE, false)) {
        cfg_prefs.putBool("wifi_en", settings.enabled);
        cfg_prefs.putUChar("bright", settings.brightness);
        cfg_prefs.putString("ssid", settings.ssid);
        cfg_prefs.putString("pass", settings.password);
        cfg_prefs.end();
    }
}

WifiSettings wifi_manager_get_settings() {
    return current_settings;
}

void wifi_manager_set_enabled(bool enabled) {
    current_settings.enabled = enabled;
    if (!enabled) {
        wifi_manager_disconnect();
    } else if (strlen(current_settings.ssid) > 0) {
        wifi_manager_connect(current_settings.ssid, current_settings.password);
    }
    wifi_manager_save_settings(current_settings);
}

void wifi_manager_connect(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return;

    strncpy(current_settings.ssid, ssid, sizeof(current_settings.ssid) - 1);
    if (pass) {
        strncpy(current_settings.password, pass, sizeof(current_settings.password) - 1);
    }
    current_settings.enabled = true;

    WiFi.mode(WIFI_STA);
    WiFi.begin(current_settings.ssid, current_settings.password);
    wifi_manager_save_settings(current_settings);
}

void wifi_manager_disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

bool wifi_manager_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifi_manager_get_ip() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return String("0.0.0.0");
}

String wifi_manager_get_status_str() {
    if (!current_settings.enabled) return String("WiFi desactivado");
    switch (WiFi.status()) {
        case WL_CONNECTED:
            return String("Conectado: ") + WiFi.localIP().toString();
        case WL_DISCONNECTED:
            return String("Desconectado");
        case WL_NO_SSID_AVAIL:
            return String("Red no encontrada");
        case WL_CONNECT_FAILED:
            return String("Fallo de clave");
        case WL_IDLE_STATUS:
            return String("Conectando...");
        default:
            return String("Buscando red...");
    }
}

int wifi_manager_scan_networks() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    return (n < 0) ? 0 : n;
}

String wifi_manager_get_scanned_ssid(int index) {
    return WiFi.SSID(index);
}

void wifi_manager_sync_ntp() {
    if (wifi_manager_is_connected()) {
        configTime(3600, 3600, "pool.ntp.org", "time.google.com");
    }
}

bool wifi_manager_is_time_synced() {
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    return (tm_info && tm_info->tm_year > (2022 - 1900));
}

String wifi_manager_get_time_str(bool include_seconds) {
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);

    if (tm_info && tm_info->tm_year > (2022 - 1900)) {
        char buf[16];
        if (include_seconds) {
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
        } else {
            snprintf(buf, sizeof(buf), "%02d:%02d", tm_info->tm_hour, tm_info->tm_min);
        }
        return String(buf);
    } else {
        // Reloj simulado a partir de millis() (empieza por defecto a las 12:00)
        uint32_t total_sec = (millis() / 1000) + (12 * 3600);
        int h = (total_sec / 3600) % 24;
        int m = (total_sec / 60) % 60;
        int s = total_sec % 60;
        char buf[16];
        if (include_seconds) {
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
        } else {
            snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
        }
        return String(buf);
    }
}

void wifi_manager_set_brightness(uint8_t brightness) {
    if (brightness < 20) brightness = 20;
    current_settings.brightness = brightness;
    tft.setBrightness(brightness);

    if (cfg_prefs.begin(CFG_NAMESPACE, false)) {
        cfg_prefs.putUChar("bright", brightness);
        cfg_prefs.end();
    }
}

uint8_t wifi_manager_get_brightness() {
    return current_settings.brightness;
}

void wifi_manager_update() {
    uint32_t now = millis();
    if (now - last_wifi_check >= 10000) {
        last_wifi_check = now;
        if (current_settings.enabled && WiFi.status() != WL_CONNECTED && strlen(current_settings.ssid) > 0) {
            WiFi.reconnect();
        }
    }
}
