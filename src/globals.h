#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// ==================== Настройки Wi-Fi (для отправки на сервер) ====================
// ВНИМАНИЕ: эти параметры используются ТОЛЬКО для подключения к сети, 
// в которой находится Django-сервер. Для сниффинга подключение не требуется.
extern const char* WIFI_SSID;         // Имя сети (пока жёстко, позже можно через меню)
extern const char* WIFI_PASSWORD;     // Пароль

extern const char* WIFI_SSID_SEND;         // Имя сети (пока жёстко, позже можно через меню)
extern const char* WIFI_PASSWORD_SEND; 

// ==================== Таймауты ====================
extern const unsigned long WIFI_CONNECT_TIMEOUT_MS;  // 20 секунд на подключение

#endif