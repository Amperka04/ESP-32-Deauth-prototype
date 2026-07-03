#include "deauth_detector.h"
#include "esp_wifi.h"
#include <WiFi.h>

// ==================== Настройки детектора ====================
// Порог срабатывания: количество Deauth-пакетов от одного BSSID за 1 секунду
#define DEAUTH_THRESHOLD 10
// Время в миллисекундах для сбора статистики
#define STATS_WINDOW_MS 1000

// ==================== Глобальные переменные модуля ====================
static DeauthCallback attackCallback = nullptr; // Callback для уведомлений
static bool isActive = false;                   // Флаг активности

// Переменные для подсчета пакетов и анализа атак
static unsigned long statsStartTime = 0;        // Время начала текущего окна
static unsigned int deauthCount = 0;            // Счетчик Deauth-пакетов
static String lastAttackerMAC = "";             // MAC атакующего из последнего пакета
static String lastTargetBSSID = "";             // BSSID из последнего пакета

// ==================== Вспомогательные функции ====================

/**
 * @brief Форматирует MAC-адрес из массива байт в строку "XX:XX:XX:XX:XX:XX".
 */
String macToString(const uint8_t* mac) {
    char buffer[18];
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buffer);
}

/**
 * @brief Проверяет, является ли кадр Deauthentication (тип 0x00, подтип 0x0C).
 * 
 * В заголовке 802.11 первые два байта — это Frame Control.
 * - Тип кадра (Type): биты 2-3 (значение 0x00 для управляющих)
 * - Подтип (Subtype): биты 4-7 (значение 0x0C для Deauth)
 * 
 * @param frameControl Первые два байта заголовка 802.11.
 * @return true, если это Deauth-кадр.
 */
bool isDeauthFrame(uint16_t frameControl) {
    // Получаем тип: (frameControl >> 2) & 0x0F
    uint8_t type = (frameControl >> 2) & 0x0F;
    // Получаем подтип: (frameControl >> 4) & 0x0F
    uint8_t subtype = (frameControl >> 4) & 0x0F;
    
    // Management frame (type=0) и Subtype=Deauthentication (0x0C = 12)
    return (type == 0 && subtype == 12);
}

// ==================== Обработчик пакетов (Sniffer) ====================

/**
 * @brief Callback-функция, которая вызывается драйвером Wi-Fi для каждого пакета.
 * 
 * Выполняется в контексте прерывания, поэтому должна быть максимально быстрой.
 * 
 * @param buf Указатель на буфер с сырыми данными пакета.
 * @param type Тип пакета (не используется).
 */
void IRAM_ATTR snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    // Структура, описывающая заголовок пакета, который пришел от драйвера
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    // Указатель на начало 802.11 заголовка (после служебных данных драйвера)
    uint8_t* payload = pkt->payload;
    
    // Минимальная длина 802.11 заголовка — 24 байта
    if (pkt->rx_ctrl.sig_len < 24) {
        return;
    }

    // Первые 2 байта — Frame Control
    uint16_t frameControl = payload[0] | (payload[1] << 8);

    // Проверяем, является ли кадр Deauth
    if (!isDeauthFrame(frameControl)) {
        return;
    }

    // Извлекаем MAC-адреса из заголовка:
    // Address 1 (получатель) — байты 4-9
    // Address 2 (отправитель) — байты 10-15
    // Address 3 (BSSID) — байты 16-21
    uint8_t* addr1 = &payload[4];
    uint8_t* addr2 = &payload[10];
    uint8_t* addr3 = &payload[16];

    // Нас интересуют пакеты, направленные НЕ на нас (broadcast или другим клиентам)
    // Это стандартный подход для детектора, чтобы не ловить свои же пакеты.
    // Однако, для простоты, мы можем обрабатывать все.
    // Оставляем эту проверку закомментированной, но показываем идею.
    /*
    if (memcmp(addr1, WiFi.macAddress().c_str(), 6) == 0) {
        return; // Это пакет для нас, игнорируем
    }
    */

    // Обновляем информацию о последнем пакете
    lastAttackerMAC = macToString(addr2); // Отправитель — атакующий
    lastTargetBSSID = macToString(addr3); // BSSID — цель атаки
    
    // Увеличиваем счетчик
    deauthCount++;

    // Отладка в Serial (можно закомментировать)
    Serial.print("[DEAUTH] От: ");
    Serial.print(lastAttackerMAC);
    Serial.print(" | Цель: ");
    Serial.println(lastTargetBSSID);
}

// ==================== Публичные функции модуля ====================

void initDeauthDetector(DeauthCallback callback) {
    if (isActive) {
        Serial.println("[Детектор] Уже активен.");
        return;
    }

    Serial.println("[Детектор] Инициализация...");
    
    // Сохраняем callback
    attackCallback = callback;
    
    // Сбрасываем счетчики
    deauthCount = 0;
    statsStartTime = millis();
    lastAttackerMAC = "";
    lastTargetBSSID = "";

    // 1. Устанавливаем режим Wi-Fi в STA (станция)
    // Это необходимо для работы promiscuous-режима
    WiFi.mode(WIFI_STA);
    delay(100);

    // 2. Настраиваем фильтр: нас интересуют только управляющие кадры (Management)
    // Это повышает эффективность, т.к. мы не обрабатываем лишние данные.
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT
    };
    esp_wifi_set_promiscuous_filter(&filter);

    // 3. Включаем promiscuous-режим и регистрируем наш callback
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(snifferCallback);
    
    isActive = true;
    Serial.println("[Детектор] Активен. Сниффинг запущен.");
}

void stopDeauthDetector() {
    if (!isActive) return;
    
    esp_wifi_set_promiscuous(false);
    isActive = false;
    attackCallback = nullptr;
    Serial.println("[Детектор] Остановлен.");
}

bool isDeauthDetectorActive() {
    return isActive;
}

// ==================== Функция для вызова в loop() ====================

/**
 * @brief Эту функцию нужно вызывать в loop() для проверки статистики.
 * 
 * Она анализирует количество пакетов за последнюю секунду и,
 * если оно превышает порог, вызывает callback.
 */
void checkForAttack() {
    if (!isActive) return;

    unsigned long currentTime = millis();
    
    // Проверяем, истекло ли окно сбора статистики (1 секунда)
    if (currentTime - statsStartTime >= STATS_WINDOW_MS) {
        // Если за это время набралось больше порога — это атака
        if (deauthCount >= DEAUTH_THRESHOLD && attackCallback != nullptr) {
            DeauthAttackInfo info;
            info.attackerMAC = lastAttackerMAC;
            info.targetBSSID = lastTargetBSSID;
            info.count = deauthCount;
            info.timestamp = currentTime;
            
            // Вызываем callback, переданный при инициализации
            attackCallback(info);
        }
        
        // Сбрасываем счетчики для нового окна
        deauthCount = 0;
        statsStartTime = currentTime;
    }
}