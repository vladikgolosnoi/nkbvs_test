#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

GenericAdapter adapter;

#ifdef ARDUINO_ARCH_AVR
constexpr int TX_PIN = 10;
constexpr int RX_PIN = -1;
constexpr unsigned long BIT_US = 20000; // длительность бита в микросекундах

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_US);

void setup() {
  Serial.begin(115200);
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
    payload += '\n';
    dev.send(payload);
    Serial.print(F("Sent: "));
    Serial.println(input);
  }
}

#endif
