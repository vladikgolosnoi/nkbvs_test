#include "led-lib.h"
#include <Arduino.h>

LedIntr::LedIntr(IGpioAdapter* adapter, int txPin, int rxPin, unsigned long bitDelayUs)
    : _adapter(adapter), _txPin(txPin), _rxPin(rxPin), _bitDurationUs(bitDelayUs) {
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
    _adapter->delayMicro(_bitDurationUs);
}

void LedIntr::send(const String& text) {
    const int bitDuration = 100; // мс на один бит (подберите под ваш фоторезистор)

    for (size_t i = 0; i < text.length(); i++) {
        uint8_t byteVal = (uint8_t)text[i];

        // стартовый бит (0)
        sendBit(0);
        delay(bitDuration);

        // MSB - от старшего к младшим битам
        for (int bit = 7; bit >= 0; bit--) {
            sendBit((byteVal >> bit) & 1);
            delay(bitDuration);
        }
        
        // стоповый бит (1)
        sendBit(1);
        delay(bitDuration * 2);  // чуть длиннее для надёжности
    }

    sendBit(0);
}

bool is_analog(int pin) {
#if defined(ESP32)
    return false;
#elif defined(ARDUINO_AVR_UNO)
    return (pin >= A0 && pin <= A5);
#else
    return false;
#endif
}

// uint8_t LedIntr::receiveBit() {
//     if (_rxPin == -1) return 0;
//     uint8_t bit;

//     if (is_analog(_rxPin)) {
//         int analogVal = analogRead(_rxPin);
//         uint8_t bit = (analogVal > _threshold) ? 1 : 0;
//     } else {
//         int val = digitalRead(_rxPin);
//         uint8_t bit = val;
//     };

//     _adapter->delayMicro(_bitDurationUs);
//     return bit;
// }

int LedIntr::receiveInt() {
    if (_rxPin == -1) return 0;

    int analogVal = analogRead(_rxPin); 

    _adapter->delayMicro(_bitDurationUs);
    return analogVal;
}

uint8_t LedIntr::receiveBit() {
    if (_rxPin == -1) return 0;

    int analogVal = analogRead(_rxPin);       // читаем АЦП
    uint8_t bit = (analogVal > _threshold) ? 1 : 0;  // пороговое преобразование

    _adapter->delayMicro(_bitDurationUs);
    return bit;
}

void LedIntr::receiveLoop(Stream& output) {
    if (_rxPin == -1) return;
    
    while (true) {
        while (receiveBit() != 0) {}

        uint8_t byteVal = 0;
        for (int bit = 0; bit < 8; bit++) {
            uint8_t b = receiveBit();
            byteVal |= (b & 1) << bit;
        }

        if (receiveBit() == 1) {
            char c = (char)byteVal;
            output.print(c);
        }
    }
}

