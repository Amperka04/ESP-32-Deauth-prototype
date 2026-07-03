#include "deauth_detector.h"
#include "esp_wifi.h"

/*
 * ====================================================================
 *  Реализация детектора deauth-атак.
 *  Включает в себя:
 *    - настройку promiscuous-режима,
 *    - callback для обработки принятых пакетов,
 *    - разбор заголовка 802.11,
 *    - подсчёт и пороговую логику,
 *    - переключение каналов (channel hopping).
 * ====================================================================
 */

// ========== Настройки детектора ==========
#define DEAUTH_THRESHOLD 10     // кол-во deauth-пакетов за секунду для срабатывания
#define DETECTOR_CHANNEL 1      // стартовый канал
#define CHANNEL_HOPPING true    // переключаться между каналами
#define MAX_CHANNEL 11          // максимальный канал

// ========== Глобальные переменные ==========
static unsigned long packetCount = 0;          // счётчик deauth-пакетов за текущую секунду
static unsigned long lastResetTime = 0;        // время последнего сброса счётчика (мс)
static unsigned long lastAttackDetectTime = 0; // время последнего обнаружения атаки (мс)
static bool attackFlag = false;                // флаг: атака обнаружена
static String attackerMAC = "";                // MAC атакующего (последний пакет)
static String targetBSSID = "";                // BSSID цели (последний пакет)
static int currentChannel = DETECTOR_CHANNEL;  // текущий канал сниффера

// ========== Структура заголовка 802.11 для Management-кадров ==========
typedef struct {
    uint16_t frameControl;   // управляющее поле (содержит тип и подтип кадра)
    uint16_t duration;       // длительность (в микросекундах)
    uint8_t addr1[6];        // адрес получателя (DA) – обычно клиент или AP
    uint8_t addr2[6];        // адрес отправителя (SA) – для deauth это атакующий
    uint8_t addr3[6];        // адрес точки доступа (BSSID) – цель атаки
    uint16_t seqControl;     // управление последовательностью
    uint8_t payload[];       // данные (для deauth здесь находится код причины)
} __attribute__((packed)) wifi_80211_mgmt_header_t;

// ========== Внутренние функции ==========

static void switchChannel() {
    if (!CHANNEL_HOPPING) return;
    currentChannel++;
    if (currentChannel > MAX_CHANNEL) currentChannel = 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    Serial.print("[Канал] Переключение на канал ");
    Serial.println(currentChannel);
}

// ========== Callback-функция ==========
/**
 * Callback-функция, вызываемая драйвером Wi-Fi для каждого принятого пакета
 * в promiscuous-режиме. Здесь мы анализируем кадры и ищем deauth.
 *
 * @param buf   – указатель на структуру wifi_promiscuous_pkt_t, содержащую данные пакета.
 * @param type  – тип пакета (WIFI_PKT_MGMT, WIFI_PKT_DATA, WIFI_PKT_CTRL).
 *                ВАЖНО: используем именно этот параметр, а не поле из pkt->rx_ctrl.
 */
static void wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    // Нас интересуют только Management-кадры (тип 0). Остальные игнорируем.
    if (type != WIFI_PKT_MGMT) return;

    // Приводим буфер к структуре promiscuous-пакета
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t* payload = pkt->payload;            // указатель на начало 802.11 заголовка
    uint16_t payloadLen = pkt->rx_ctrl.sig_len; // длина пакета

    // Проверяем, что длина достаточна для заголовка management-кадра (минимум 24 байта)
    if (payloadLen < sizeof(wifi_80211_mgmt_header_t)) return;

    // Приводим сырые данные к структуре заголовка
    wifi_80211_mgmt_header_t* header = (wifi_80211_mgmt_header_t*)payload;
    uint16_t frameControl = header->frameControl;

    // === Парсинг frameControl (см. стандарт 802.11) ===
    // Биты 2-3: тип кадра (0 = Management, 1 = Control, 2 = Data)
    uint8_t frameType = (frameControl & 0x000C) >> 2;
    // Биты 4-7: подтип (для Management: 0x0C = Deauth, 0x0A = Disassoc)
    uint8_t subType = (frameControl & 0x00F0) >> 4;

    // Если это не Management-кадр – выходим
    if (frameType != 0) return;
    // Если подтип не Deauth (0x0C) и не Disassoc (0x0A) – выходим
    if (subType != 0x0C && subType != 0x0A) return;

    // Парсинг MAC-адресов
    // addr2 – отправитель (атакующий), addr3 – BSSID (цель)
    char macStr[18];

    // Форматируем MAC атакующего (addr2)
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             header->addr2[0], header->addr2[1], header->addr2[2],
             header->addr2[3], header->addr2[4], header->addr2[5]);
    String currentAttacker = String(macStr);

    // Форматируем BSSID цели (addr3)
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             header->addr3[0], header->addr3[1], header->addr3[2],
             header->addr3[3], header->addr3[4], header->addr3[5]);
    String currentTarget = String(macStr);

    // Логирование каждого пакета в Serial (для отладки, можно сделать вывод в txt файлик через отдельный метод для логгирования наверное)
    Serial.print("[DEAUTH] Attacker: ");
    Serial.print(currentAttacker);
    Serial.print(" | Target: ");
    Serial.print(currentTarget);
    Serial.print(" | Channel: ");
    Serial.println(currentChannel);

    // Увеличиваем счётчик пакетов и сохраняем последние MAC-адреса
    packetCount++;
    attackerMAC = currentAttacker;
    targetBSSID = currentTarget;
}

// ========== Публичные функции ==========

void initDeauthDetector() {
    Serial.println("[Detector] Инициализация...");

    // 1. Отключаем Wi-Fi от текущей сети, чтобы освободить драйвер.
    //    Это необходимо для корректной работы promiscuous-режима без помех.
    //    Если в будущем потребуется отправлять данные через Telegram,
    //    придётся временно выходить из promiscuous, подключаться к сети,
    //    отправлять сообщение и снова входить в promiscuous.
    WiFi.disconnect(true);  // отключаемся от текущей сети
    WiFi.mode(WIFI_MODE_STA);
    delay(100);

    // 2. Включаем promiscuous-режим (перехват всех пакетов)
    esp_wifi_set_promiscuous(true);

    // 3. Фильтр: только Management-кадры
    wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT };
    esp_wifi_set_promiscuous_filter(&filter);

    // 4. Устанавливаем callback
    esp_wifi_set_promiscuous_rx_cb(wifiSnifferCallback);

    // 5. Устанавливаем начальный канал
    esp_wifi_set_channel(DETECTOR_CHANNEL, WIFI_SECOND_CHAN_NONE);
    currentChannel = DETECTOR_CHANNEL;

    packetCount = 0;
    lastResetTime = millis();
    attackFlag = false;

    Serial.println("[Detector] Запущен на канале " + String(currentChannel));
}

DeauthAttackInfo updateDeauthDetector() {
    unsigned long currentTime = millis();
    DeauthAttackInfo info;

    // Каждую секунду выполняем проверку счётчика
    if (currentTime - lastResetTime >= 1000) {
        // Если количество пакетов за секунду превысило порог – фиксируем атаку
        if (packetCount >= DEAUTH_THRESHOLD) {
            attackFlag = true;
            lastAttackDetectTime = currentTime;
            Serial.println("=== ВНИМАНИЕ: ОБНАРУЖЕНА DEAUTH-АТАКА! ===");
            Serial.print("Пакетов: "); Serial.println(packetCount);
            Serial.print("Атакующий MAC: "); Serial.println(attackerMAC);
            Serial.print("Целевой BSSID: "); Serial.println(targetBSSID);
        } else {
            // Если атака была, но новых пакетов нет более 5 секунд – сбрасываем флаг
            if (attackFlag && (currentTime - lastAttackDetectTime > 5000)) {
                attackFlag = false;
                Serial.println("[Detector] Атака завершена.");
            }
        }

        packetCount = 0;
        lastResetTime = currentTime;
        switchChannel();  // переключаем канал каждую секунду
    }

    // Заполняем структуру для возврата
    info.attackDetected = attackFlag;
    info.attackerMAC = attackerMAC;
    info.targetBSSID = targetBSSID;
    info.packetCount = packetCount;
    info.lastAttackTime = lastAttackDetectTime;
    return info;
}

bool isAttackDetected() {
    return attackFlag;
}

DeauthAttackInfo getLastAttackInfo() {
    DeauthAttackInfo info;
    info.attackDetected = attackFlag;
    info.attackerMAC = attackerMAC;
    info.targetBSSID = targetBSSID;
    info.packetCount = packetCount;
    info.lastAttackTime = lastAttackDetectTime;
    return info;
}

uint8_t getCurrentChannel() {
    return currentChannel;
}