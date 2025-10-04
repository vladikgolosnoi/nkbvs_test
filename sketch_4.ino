#include <Arduino.h>
#include "Arduino-lib.h"
#include "led-lib.h"

ArduinoAdapter adapter;
// txPin = 13, без rx
LedIntr dev(&adapter, 13, -1, 500);

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  dev.send("HELLO\n");
  Serial.println("Sent: HELLO");
}
