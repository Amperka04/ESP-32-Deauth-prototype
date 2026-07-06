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
static unsigned long packetCount = 0;
static unsigned long lastResetTime = 0;
static unsigned long lastAttackDetectTime = 0;
static bool attackFlag = false;
static String attackerMAC = "";
static String targetBSSID = "";
static int currentChannel = DETECTOR_CHANNEL;

// ========== Структура заголовка 802.11 ==========
typedef struct {
    uint16_t frameControl;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seqControl;
    uint8_t payload[];
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
static void wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t* payload = pkt->payload;
    uint16_t payloadLen = pkt->rx_ctrl.sig_len;

    if (payloadLen < sizeof(wifi_80211_mgmt_header_t)) return;

    wifi_80211_mgmt_header_t* header = (wifi_80211_mgmt_header_t*)payload;
    uint16_t frameControl = header->frameControl;

    uint8_t frameType = (frameControl & 0x000C) >> 2;
    uint8_t subType = (frameControl & 0x00F0) >> 4;

    if (frameType != 0) return;
    if (subType != 0x0C && subType != 0x0A) return;

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             header->addr2[0], header->addr2[1], header->addr2[2],
             header->addr2[3], header->addr2[4], header->addr2[5]);
    String currentAttacker = String(macStr);

    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             header->addr3[0], header->addr3[1], header->addr3[2],
             header->addr3[3], header->addr3[4], header->addr3[5]);
    String currentTarget = String(macStr);

    Serial.print("[DEAUTH] Attacker: ");
    Serial.print(currentAttacker);
    Serial.print(" | Target: ");
    Serial.print(currentTarget);
    Serial.print(" | Channel: ");
    Serial.println(currentChannel);

    packetCount++;
    attackerMAC = currentAttacker;
    targetBSSID = currentTarget;
}

// ========== Публичные функции ==========

void initDeauthDetector() {
    Serial.println("[Detector] Инициализация...");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    delay(100);

    esp_wifi_set_promiscuous(true);

    wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT };
    esp_wifi_set_promiscuous_filter(&filter);

    esp_wifi_set_promiscuous_rx_cb(wifiSnifferCallback);

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

    if (currentTime - lastResetTime >= 1000) {
        if (packetCount >= DEAUTH_THRESHOLD) {
            attackFlag = true;
            lastAttackDetectTime = currentTime;
            Serial.println("=== ВНИМАНИЕ: ОБНАРУЖЕНА DEAUTH-АТАКА! ===");
            Serial.print("Пакетов: "); Serial.println(packetCount);
            Serial.print("Атакующий MAC: "); Serial.println(attackerMAC);
            Serial.print("Целевой BSSID: "); Serial.println(targetBSSID);
        } else {
            if (attackFlag && (currentTime - lastAttackDetectTime > 5000)) {
                attackFlag = false;
                Serial.println("[Detector] Атака завершена.");
            }
        }

        packetCount = 0;
        lastResetTime = currentTime;
        switchChannel();
    }

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