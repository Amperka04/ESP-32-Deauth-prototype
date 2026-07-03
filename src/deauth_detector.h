#ifndef DEAUTH_DETECTOR_H
#define DEAUTH_DETECTOR_H

#include <Arduino.h>
#include <WiFi.h>

/*
 * ====================================================================
 *  Модуль детектирования deauth-атак с использованием прослушивающего режима
 *  Основная логика:
 *    - перехват всех Wi-Fi-кадров,
 *    - фильтрация Management-кадров с подтипом Deauth (0x0C) или Disassoc (0x0A),
 *    - подсчёт количества таких пакетов за секунду,
 *    - если превышен порог (DEAUTH_THRESHOLD) – фиксируется атака.
 * ====================================================================
 */

// Структура для хранения информации об обнаруженной атаке Deauth
struct DeauthAttackInfo {
    bool attackDetected;        // true если атака обнаружена
    String attackerMAC;         // MAC-адрес атакующего
    String targetBSSID;         // BSSID (MAC) сети-жертвы
    unsigned int packetCount;   // количество пакетов за последнюю секунду
    unsigned long lastAttackTime; // время последнего обнаружения (мс)
};

/**
 * Инициализация детектора deauth-пакетов.
 * Включает прослушивающий режим и устанавливает callback-функцию.
 * Должна быть вызвана один раз в setup() после инициализации Wi-Fi.
 */
void initDeauthDetector();

/**
 * Обновление информации о состоянии детектора.
 * Должна вызываться в loop() как можно чаще.
 * @return DeauthAttackInfo - структура с текущим статусом атаки
 */
DeauthAttackInfo updateDeauthDetector();

/**
 * Проверка, обнаружена ли атака в данный момент.
 * @return true если атака активна
 */
bool isAttackDetected();

/**
 * Получение последней информации об атаке.
 * @return DeauthAttackInfo - структура с данными об атаке
 */
DeauthAttackInfo getLastAttackInfo();

/**
 * Возвращает текущий номер Wi-Fi канала, на котором работает сниффер
 * @return номер канала (1..11)
 */
uint8_t getCurrentChannel();

#endif