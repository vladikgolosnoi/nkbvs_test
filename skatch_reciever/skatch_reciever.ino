#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

GenericAdapter adapter;

#ifdef ARDUINO_ARCH_ESP32
constexpr int TX_PIN = -1;
constexpr int RX_PIN = 34;
constexpr unsigned long BIT_US = 20000; // длительность бита в микросекундах

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_US);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println(F("--- ESP32 LED receiver ready ---"));
  dev.setSamplesPerBit(5);
  Serial.print(F("Threshold: "));
  Serial.println(dev.threshold());
}

void loop() {
  String received;
  if (dev.receiveMessage(received, 64, 2000)) {
    received.trim();
    Serial.print(F("RX: "));
    Serial.println(received);
  } else {
    delay(5);
  }
}

#endif
