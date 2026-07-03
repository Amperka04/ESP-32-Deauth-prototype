#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// ==================== Настройки Wi-Fi ====================
extern const char* WIFI_SSID;         // Имя сети (пока жёстко, позже можно через меню)
extern const char* WIFI_PASSWORD;     // Пароль

// ==================== Таймауты ====================
extern const unsigned long WIFI_CONNECT_TIMEOUT_MS;  // 20 секунд на подключение

#endif