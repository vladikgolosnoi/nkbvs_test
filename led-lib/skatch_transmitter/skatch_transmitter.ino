#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

// Настройки
#define TX_PIN 10
#define RX_PIN -1
#define BIT_DURATION 500  // миллисекунды

GenericAdapter adapter;

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_DURATION);

uint8_t message[] = { 0b10101010, 0b11001100 }; // пример пакета

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Transmitter started");
}

void loop() {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      if (input.length() == 0) {
        return;
      }

      Serial.print(F("Получено: "));
      Serial.println(input);

      String payload = input;

      dev.sendPacket(message, sizeof(message));
      Serial.print(F("Отправлено: "));
      Serial.println(input);
    }
    // Serial.println("Sending packet...");
    
    // delay(2000); // пауза между отправками
}
