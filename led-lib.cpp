#include "led-lib.h"
#include <Arduino.h>

LedIntr::LedIntr(IGpioAdapter* adapter, int txPin, int rxPin, unsigned long bitDelayUs)
    : _adapter(adapter), _txPin(txPin), _rxPin(rxPin), _bitDelay(bitDelayUs) {
    if (_txPin != -1) {
        pinMode(_txPin, OUTPUT);
    }
    if (_rxPin != -1) {
        pinMode(_rxPin, INPUT);
    }
}

void LedIntr::sendBit(uint8_t bit) {
    if (_txPin == -1) return;

    _adapter->pinWrite(_txPin, bit ? HIGH : LOW);
    _adapter->delay(_bitDelay);
}

void LedIntr::send(const String& text) {
    for (size_t i = 0; i < text.length(); i++) {
        uint8_t byteVal = (uint8_t)text[i];

        // стартовый бит (0)
        sendBit(0);
        
        // отправляем все 8 бит данных
        for (int bit = 0; bit < 8; bit++) {
            sendBit((byteVal >> bit) & 1);
        }
        
        // стоповый бит (1)
        sendBit(1);
    }
}

uint8_t LedIntr::receiveBit() {
    if (_rxPin == -1) return 0;

    int analogVal = analogRead(_rxPin);
    uint8_t bit = (analogVal > _threshold) ? 1 : 0;

    _adapter->delayMicro(_bitDelay);
    return bit;
}

void LedIntr::receiveLoop() {
    if (_rxPin == -1) return;
    
    while (true) {
        // ждем стартовый бит (0)
        while (receiveBit() != 0) {
            // ожидание...
        }
        
        uint8_t byteVal = 0;

        // читаем 8 бит данных
        for (int bit = 0; bit < 8; bit++) {
            uint8_t b = receiveBit();
            byteVal |= (b & 1) << bit;
        }

        // проверяем стоповый бит (должен быть 1)
        if (receiveBit() == 1) {
            char c = (char)byteVal;
            Serial.print(c);  // сразу выводим в Serial
        }
    }
}
