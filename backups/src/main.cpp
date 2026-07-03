#include <Arduino.h>
#include "display.h"
#include "globals.h"
#include "wifi_manager.h"
#include "deauth_detector.h"

// ========== Определение глобальных констант (из globals.h) ==========
const char* WIFI_SSID = "TP-Link_FEAB";          // Замените на свои данные
const char* WIFI_PASSWORD = "68825355";
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000; // 20 секунд



void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\n=== Запуск Deauth Detector ===");

    // 1. Инициализация дисплея
    initDisplay(); 
    
    // 2. Подключение к Wi-Fi
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Connecting...");

    if (connectToWiFi()) {
        // Успех – выводим на экран
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 10);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextSize(3);
        tft.println("Connected!");
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 60);
        tft.print("IP: ");
        tft.println(getLocalIP());
        delay(2000);
    } else {
        // Ошибка – выводим красным
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 10);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(3);
        tft.println("WiFi Error");
        tft.setTextSize(2);
        tft.setCursor(10, 60);
        tft.println("Check SSID/PASSWORD");
        delay(5000);
    }

    // 3. (Опционально) Сканирование сетей для отладки – выведем в Serial
    // scanWiFiNetworks();
}

void loop() {
    // Пока ничего не делаем. Позже здесь будет сниффинг.
    delay(1000);
}