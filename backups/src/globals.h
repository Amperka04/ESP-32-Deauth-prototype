#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// ==================== Настройки Wi-Fi ====================
extern const char* WIFI_SSID;         // Имя сети (пока жёстко, позже можно через меню)
extern const char* WIFI_PASSWORD;     // Пароль

// ==================== Таймауты ====================
extern const unsigned long WIFI_CONNECT_TIMEOUT_MS;  // 20 секунд на подключение

// ==================== Настройки Deauth Детектора ====================
extern const uint8_t DEAUTH_CHANNEL_START;      // Стартовый канал (1)
extern const uint8_t DEAUTH_CHANNEL_MAX;        // Максимальный канал (11 для US, 13 для EU)
extern const bool DEAUTH_CHANNEL_HOPPING;       // Переключать каналы автоматически
extern const unsigned int DEAUTH_SCAN_TIME_MS;  // Время сканирования на канале (мс)
extern const unsigned int DEAUTH_PACKET_RATE;   // Порог пакетов/сек для атаки
extern const uint8_t DEAUTH_LED_PIN;            // Пин для LED-индикации (опционально)

#endif