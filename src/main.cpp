#include <Arduino.h>
#include "display.h"
#include "globals.h"
#include "wifi_manager.h"
#include "deauth_detector.h"

// ДЛЯ ДОБАВЛЕНИЯ TELEGRAM-ОТПРАВКИ:
// В функции loop() внутри блока if (info.attackDetected) вызовите
// свою функцию sendTelegramAlert(), передав ей информацию об атаке.
// ВНИМАНИЕ: перед отправкой нужно временно выйти из promiscuous-режима,
// подключиться к Wi-Fi, отправить сообщение, затем снова войти в promiscuous.
// См. пример в комментариях ниже.

// ========== Определение глобальных констант (из globals.h) ==========
const char* WIFI_SSID = ""; //имя сети       
const char* WIFI_PASSWORD = ""; //пароль
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;



// ========== Настройка и главный цикл ==========

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\n=== Запуск Deauth Detector (Pure Sniffer) ===");

    // 1. Инициализация дисплея
    initDisplay();
    
    // 2. Показываем стартовый экран
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

    // 3. Инициализация детектора (отключает STA, включает сниффинг)
    initDeauthDetector();
    
    // Очищаем экран и показываем статус
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
    // Обновляем состояние детектора
    DeauthAttackInfo info = updateDeauthDetector();
    
    if (info.attackDetected) {
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

        // ========== ВСТАВИТЬ ОТПРАВКУ В TELEGRAM ==========
        // TODO: Вызвать функцию отправки уведомления в Telegram.
        // Пример:
        // if (sendTelegramAlert(info.attackerMAC, info.targetBSSID, info.packetCount)) {
        //     Serial.println("[Telegram] Уведомление отправлено.");
        // }
        // ВАЖНО: перед отправкой необходимо:
        //   1. Выйти из promiscuous-режима: esp_wifi_set_promiscuous(false);
        //   2. Подключиться к Wi-Fi (connectToWiFi()).
        //   3. Отправить сообщение через HTTPS POST к API Telegram.
        //   4. Отключиться от Wi-Fi и снова войти в promiscuous (initDeauthDetector()).
        // ==================================================


        // можно добавить файлы типо telegram_notifier.h и telegram_notifier.cpp для ТГ бота
        // Также нужно будет добавить библиотеку в файле platformio.ini для взаимодействия с ТГ, 
        // например FastBot2 (предпочтительнее) или UniversalTelegramBot 

    } else {
        // Показываем статус и текущий канал (обновляем раз в секунду)
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
    delay(100); // Небольшая задержка для снижения нагрузки на процессор
}