#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * Подключает ESP32 к заданной Wi-Fi сети (SSID и пароль берутся из globals.h).
 * @return true, если подключение успешно, иначе false.
 */
bool connectToWiFi();


bool connectToWiFiForSend();
/*
 * Сканирует доступные Wi-Fi сети и выводит их в Serial (опционально).
 * Можно использовать для отладки или для вывода списка сетей на экран.
 */
void scanWiFiNetworks();

/**
 * Возвращает текущий IP-адрес ESP32 в виде строки.
 * @return String с IP-адресом (например, "192.168.1.100").
 */
String getLocalIP();

#endif