#include "alert_notifier.h"
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
        Serial.println("[Alert] Wi-Fi не подключён. Подключаюсь...");
        if (!connectToWiFi()) {
            Serial.println("[Alert] Ошибка подключения к Wi-Fi. Отправка невозможна.");
            return;
        }
    }

#if TEST_MODE
    // ========== ТЕСТОВЫЙ РЕЖИМ ==========
    Serial.println("[Alert] ========== ТЕСТОВЫЙ РЕЖИМ ==========");
    Serial.println("[Alert] Формирование JSON для отправки:");
    
    // Формируем JSON (для показа в Serial)
    String json = "{";
    json += "\"attacker_mac\":\"" + String(record.attackerMAC) + "\",";
    json += "\"target_bssid\":\"" + String(record.targetBSSID) + "\",";
    json += "\"packet_count\":" + String(record.packetCount);
    json += "}";
    
    Serial.println("[Alert] Отправка JSON: " + json);
    delay(500);
    Serial.println("[Alert] Атака сохранена в Django!");

#else
    HTTPClient http;
    WiFiClient client;
    // ссылка для отправки информации на сайт с django
    String url = "http://10.111.31.250:8000/api/add_attack/";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    
    // Формируем JSON
    String json = "{";
    json += "\"attacker_mac\":\"" + String(record.attackerMAC) + "\",";
    json += "\"target_bssid\":\"" + String(record.targetBSSID) + "\",";
    json += "\"packet_count\":" + String(record.packetCount);
    json += "}";
    
    Serial.println("[Alert] Отправка JSON: " + json);
    
    int code = http.POST(json);
    if (code == 201) {
        Serial.println("[Alert] Атака сохранена в Django!");
    } else {
        Serial.println("[Alert] Ошибка: " + String(code) + " - " + http.errorToString(code));
    }
    http.end();

#endif
}