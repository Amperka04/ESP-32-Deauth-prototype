#include "display.h"
#include <Arduino.h>

// Определение глобального объекта дисплея
TFT_eSPI tft = TFT_eSPI();

/*
 * Инициализация дисплея.
 * Включает питание LCD, настраивает ШИМ для подсветки, инициализирует TFT_eSPI,
 * устанавливает ориентацию и выводит тестовое сообщение.
 */
void initDisplay() {
    // Включаем питание LCD (пин 15)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    // Настраиваем ШИМ для подсветки (пин 38)
    ledcSetup(0, 5000, 8);      // канал 0, частота 5 кГц, 8 бит
    ledcAttachPin(38, 0);       // привязываем пин 38 к каналу 0
    ledcWrite(0, 255);          // начальная яркость 200 (из 255)

    // Инициализация библиотеки TFT_eSPI
    tft.init();
    tft.setRotation(3);         // Ориентация экрана: 3 — подобрана пользователем
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(10, 10);
    tft.println("Display OK!"); // Тестовое сообщение
}