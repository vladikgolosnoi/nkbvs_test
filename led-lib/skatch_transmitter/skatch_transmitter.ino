#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

GenericAdapter adapter;

#ifdef ARDUINO_ARCH_AVR
constexpr int TX_PIN = 10;
constexpr int RX_PIN = -1;
constexpr unsigned long BIT_US = 20000; // длительность бита в микросекундах
int rec;

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_US);

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    Serial.print("Получено: ");
    Serial.println(input);

    dev.send(input);
    Serial.print("Sent: ");
    Serial.println(input);
  }
}

#endif

