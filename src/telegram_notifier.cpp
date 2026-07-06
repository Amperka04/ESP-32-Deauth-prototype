#include "telegram_notifier.h"
#include "globals.h"
#include "wifi_manager.h"
#include <HTTPClient.h>
#include <WiFiClient.h>

/*
 * ====================================================================
 *  Реализация отправки данных на Django-сервер.
 *  Перед отправкой подключается к Wi-Fi (если не подключён).
 * ====================================================================
 */

void sendToDjango(DeauthAttackInfo record) {
    // Проверяем подключение к Wi-Fi, при необходимости подключаемся
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Telegram] Wi-Fi не подключён. Подключаюсь...");
        if (!connectToWiFi()) {
            Serial.println("[Telegram] Ошибка подключения к Wi-Fi. Отправка невозможна.");
            return;
        }
    }

    HTTPClient http;
    WiFiClient client;
    
    String url = "http://10.111.31.250:8000/api/add_attack/";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    
    // Формируем JSON (поле channel пока отсутствует)
    String json = "{";
    json += "\"attacker_mac\":\"" + String(record.attackerMAC) + "\",";
    json += "\"target_bssid\":\"" + String(record.targetBSSID) + "\",";
    json += "\"packet_count\":" + String(record.packetCount);
    json += "}";
    
    Serial.println("[Telegram] Отправка JSON: " + json);
    
    int code = http.POST(json);
    if (code == 201) {
        Serial.println("[Telegram] Атака сохранена в Django!");
    } else {
        Serial.println("[Telegram] Ошибка: " + String(code) + " - " + http.errorToString(code));
    }
    http.end();
}