#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

struct WifiSettings {
    bool enabled;
    char ssid[33];
    char password[65];
    uint8_t brightness; // 20 - 255
};

void wifi_manager_init();
void wifi_manager_save_settings(const WifiSettings& settings);
WifiSettings wifi_manager_get_settings();

void wifi_manager_set_enabled(bool enabled);
void wifi_manager_connect(const char* ssid, const char* pass);
void wifi_manager_disconnect();

bool wifi_manager_is_connected();
String wifi_manager_get_ip();
String wifi_manager_get_status_str();

int wifi_manager_scan_networks();
String wifi_manager_get_scanned_ssid(int index);

void wifi_manager_sync_ntp();
bool wifi_manager_is_time_synced();
String wifi_manager_get_time_str(bool include_seconds = false);

void wifi_manager_set_brightness(uint8_t brightness);
uint8_t wifi_manager_get_brightness();

void wifi_manager_update();

#endif // WIFI_MANAGER_H
