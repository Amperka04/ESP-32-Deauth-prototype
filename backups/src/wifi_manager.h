#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * Подключает ESP32 к заданной Wi-Fi сети (SSID и пароль берутся из globals.h).
 * @return true, если подключение успешно, иначе false.
 */
bool connectToWiFi();

/**
 * Сканирует доступные Wi-Fi сети и выводит их в Serial (опционально).
 * Можно использовать для отладки или для вывода списка на экран.
 */
void scanWiFiNetworks();

/**
 * Получить текущий IP-адрес (строка).
 */
String getLocalIP();

#endif