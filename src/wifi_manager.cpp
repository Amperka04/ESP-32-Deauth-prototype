#include "wifi_manager.h"
#include "globals.h"

/*
 * ====================================================================
 *  Реализация функций для подключения к Wi-Fi
 *  В текущей версии проекта Wi-Fi отключается перед включением сниффера,
 *  но эти функции оставлены для будущего использования (Telegram бот)
 * ====================================================================
 */

bool connectToWiFi() {
    Serial.print("Подключение к Wi-Fi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA); // режим станции (клиента)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Ждём подключения до истечения таймаута
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Подключено!");
        Serial.print("IP-адрес: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\n❌ Ошибка подключения. Проверьте SSID/пароль.");
        return false;
    }
}

void scanWiFiNetworks() {
    Serial.println("Сканирование Wi-Fi сетей...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("Сетей не найдено.");
    } else {
        Serial.print("Найдено ");
        Serial.print(n);
        Serial.println(" сетей:");
        for (int i = 0; i < n; i++) {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.println(" dBm)");
        }
    }
    WiFi.scanDelete(); // освобождаем память после сканирования
}

String getLocalIP() {
    return WiFi.localIP().toString();
}