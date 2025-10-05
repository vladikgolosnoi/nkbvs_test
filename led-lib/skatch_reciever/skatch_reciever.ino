#include <Arduino.h>
#include "Platform-lib.h"
#include "led-lib.h"

// Настройки
#define TX_PIN -1
#define RX_PIN 34
#define BIT_DURATION 1000  // миллисекунды
#define THRESHOLD 1000

uint8_t serviceCode[5] = {1, 1, 1, 1, 1}; // код остановки транспорта сообщ

int calibrateDur = 50;
int calibrateValue = 0;

int currentValue;
int previousValue;

int bit;

bool isChanged;

GenericAdapter adapter;

LedIntr dev(&adapter, TX_PIN, RX_PIN, BIT_DURATION);

//uint8_t buffer[16];

void setup() {
    Serial.begin(115200);
    dev.setThreshold(THRESHOLD);
    delay(1000);
    Serial.println("Receiver started");
}

void loop() {
    calibrate();

    // dev.receivePacket(buffer, sizeof(buffer));

    bit = dev.receiveBit();
    Serial.print("Bit received: ");
    Serial.println(bit);
}

void calibrate() {
    while (true) {
        previousValue = currentValue;
        int calibrateValue = currentValue = dev.receiveBitRaw(calibrateDur);
        Serial.println(calibrateValue);

        isChanged = (currentValue - previousValue >= THRESHOLD) ? 1 : 0;

        if (isChanged) { break; }
    }
}

// void loop() {
//     if (Serial.available() > 0) {
//       String input = Serial.readStringUntil('\n');
//       if (input.length() == 0) {
//         return;
//       }

//       Serial.print(F("Получено: "));
//       Serial.println(input);

//       String payload = input;

//       dev.sendPacket(message, sizeof(message));
//       Serial.print(F("Sent: "));
//       Serial.println(input);
//     }
//     // Serial.println("Sending packet...");
    
//     // delay(2000); // пауза между отправками
// }

        // for (int i = 0; i < 2; i++) { // выводим первые 2 байта для примера
        //     Serial.print(buffer[i], BIN);
        //     Serial.print(" ");
        // }
