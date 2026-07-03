#ifndef DEAUTH_DETECTOR_H
#define DEAUTH_DETECTOR_H

#include <Arduino.h>

/**
 * @file deauth_detector.h
 * @brief Модуль для обнаружения Deauth-атак в режиме promiscuous.
 * 
 * ESP32 перехватывает все Wi-Fi пакеты в эфире, фильтрует управляющие кадры
 * и ищет среди них Deauthentication (0xC0). При обнаружении атаки (серии пакетов)
 * вызывается callback-функция, которую вы задаете.
 */

// Структура для хранения информации об обнаруженной атаке
struct DeauthAttackInfo {
    String attackerMAC;   // MAC-адрес атакующего (Source)
    String targetBSSID;   // BSSID целевой сети
    unsigned int count;   // Количество пакетов за последнюю секунду
    unsigned long timestamp; // Время обнаружения (мс)
};

// Тип callback-функции, которая будет вызвана при обнаружении атаки.
// Принимает структуру с информацией об атаке.
typedef void (*DeauthCallback)(DeauthAttackInfo info);

/**
 * @brief Инициализирует детектор Deauth-атак.
 * 
 * Включает promiscuous-режим, устанавливает фильтр на управляющие кадры
 * и регистрирует callback для уведомлений.
 * 
 * @param callback Указатель на функцию, которая будет вызвана при атаке.
 *                 Может быть nullptr, если уведомления не нужны.
 */
void initDeauthDetector(DeauthCallback callback);

/**
 * @brief Останавливает детектор и выключает promiscuous-режим.
 */
void stopDeauthDetector();

/**
 * @brief Проверяет, активен ли детектор.
 * @return true, если promiscuous-режим включен.
 */
bool isDeauthDetectorActive();

#endif