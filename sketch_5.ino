#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

GenericAdapter adapter;

constexpr int TX_PIN = -1;         // отключаем передачу, ESP32 работает как приёмник
constexpr int RX_PIN = 34;         // фоторезисторный делитель на входе ADC1_CH6
constexpr unsigned long BIT_US = 20000; // длительность бита в микросекундах (~833 бод)
int rec;

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_US);

void setup() {
  Serial.begin(115200);

  Serial.println(F("--- ESP32 LED receiver ready ---"));
  // Serial.print(F("Threshold: "));
  // Serial.println(dev.threshold());
}

void loop() {
  rec = (int)dev.receiveInt();

  //if (dev.receiveMessage(received, 64, 2)) {
  //  Serial.print(F("RX: "));
  //  Serial.println(received);
  //}
  Serial.println(rec);

  delay(100);
}
