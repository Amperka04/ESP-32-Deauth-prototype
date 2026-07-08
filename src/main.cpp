#include <Arduino.h>
#include "display.h"
#include "globals.h"
#include "wifi_manager.h"
#include "deauth_detector.h"
#include "telegram_notifier.h"
#include "esp_wifi.h"

// ========== Определение глобальных констант ==========
 const char* WIFI_SSID_SEND = "iPhone (97)";          // SSID сети, где находится Django
 const char* WIFI_PASSWORD_SEND = "12345678";   // Пароль

const char* WIFI_SSID = "vivo X200 Ultra";          // SSID сети, где находится Django
const char* WIFI_PASSWORD = "hyybtvrqknwm58e";   // Пароль

const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;

// Флаг для однократной отправки за одну атаку
static bool dataSent = false;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\n=== Запуск Deauth Detector (Pure Sniffer) ===");

    initDisplay();

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Sniffer");
    tft.setTextSize(1);
    tft.setCursor(10, 50);
    tft.println("Monitoring WiFi");
    tft.setCursor(10, 70);
    tft.println("for Deauth attacks");
    tft.setCursor(10, 90);
    tft.println("(No WiFi connection)");
    delay(2000);

    initDeauthDetector();

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Sniffing...");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 50);
    tft.println("Channel hopping");
    tft.setCursor(10, 70);
    tft.println("Press reset to stop");
}

void loop() {
    DeauthAttackInfo info = updateDeauthDetector();

    if (info.attackDetected) {
        // Отображаем предупреждение на экране
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(3);
        tft.setCursor(10, 10);
        tft.println("ATTACK!");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, 60);
        tft.print("From: ");
        tft.println(info.attackerMAC);
        tft.setCursor(10, 80);
        tft.print("Target: ");
        tft.println(info.targetBSSID);
        tft.setCursor(10, 100);
        tft.print("Packets: ");
        tft.println(info.packetCount);

        // Отправляем данные только один раз за атаку
        if (!dataSent) {
            Serial.println("[Main] Обнаружена атака! Выход из promiscuous...");
            esp_wifi_set_promiscuous(false);
            delay(200); // даём время на переключение режима

            Serial.println("[Main] Отправка данных на Django...");
            connectToWiFi();
            sendToDjango(info);
            Serial.println("[Main] Отправка завершена.");

            // Отключаем Wi-Fi, чтобы не мешал, и возвращаемся в promiscuous
            WiFi.disconnect(true);
            delay(100);
            esp_wifi_set_promiscuous(true);
            Serial.println("[Main] Promiscuous-режим восстановлен.");
            dataSent = true;
        }
    } else {
        // Если атака закончилась, сбрасываем флаг для следующей атаки
        if (dataSent) {
            dataSent = false;
        }

        // Показываем статус сканирования
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate > 1000) {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(10, 10);
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.setTextSize(2);
            tft.println("Scanning...");
            tft.setTextSize(1);
            tft.setCursor(10, 50);
            tft.print("Channel: ");
            tft.println(getCurrentChannel());
            tft.setCursor(10, 70);
            tft.println("Monitoring WiFi");
            tft.setCursor(10, 90);
            tft.println("for Deauth attacks");
            lastUpdate = millis();
        }
    }
    delay(100);
}