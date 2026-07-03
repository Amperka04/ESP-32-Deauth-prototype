#include "wifi_manager.h"
#include "globals.h"

// Определение глобальных переменных (если они объявлены extern в globals.h)
// Но мы определим их в main.cpp, поэтому здесь они не нужны.

bool connectToWiFi() {
    Serial.print("Подключение к Wi-Fi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    WiFi.scanDelete(); // освобождаем память
}

String getLocalIP() {
    return WiFi.localIP().toString();
}