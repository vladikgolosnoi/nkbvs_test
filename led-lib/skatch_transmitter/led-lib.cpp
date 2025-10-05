#include "led-lib.h"
#include <Arduino.h>

LedIntr::LedIntr(IGpioAdapter* adapter, int txPin, int rxPin, int bitDelay)
    : _adapter(adapter), _txPin(txPin), _rxPin(rxPin), _bitDuration(bitDelay) {
    if (_txPin != -1) {
        pinMode(_txPin, OUTPUT);
        // digitalWrite(_txPin, HIGH);
    }
    if (_rxPin != -1) {
        pinMode(_rxPin, INPUT);
    }
}

// Отправка и кодировка

void LedIntr::sendBit(uint8_t bit) {
    if (_txPin == -1) return;

    _adapter->pinWrite(_txPin, bit ? HIGH : LOW);
    _adapter->delay(_bitDuration);
}

void LedIntr::sendBitNonBlocking(uint8_t bit) {
    if (_txPin == -1) return;

    _adapter->pinWrite(_txPin, bit ? HIGH : LOW);
}

void LedIntr::sendGreetMes() {
    for (int i = 0; i < 3; i++) {
        sendBit(1);
    }

    //sendBit(0); // старт-бит
}

void LedIntr::sendPacket(const uint8_t* data, int length) {
    sendGreetMes();

    int onesCount = 0;

    for (int byteIdx = 0; byteIdx < length; byteIdx++) {
        for (int bitPos = 7; bitPos >= 0; bitPos--) {
            bool bit = (data[byteIdx] >> bitPos) & 1;
            sendBit(bit);

            if (bit) {
                onesCount++;
                if (onesCount == 5) {
                    sendBit(0);
                    _adapter->delay(_bitDuration);
                    onesCount = 0;
                }
            } else {
                onesCount = 0;
            }
        }
    }
    //digitalWrite(_txPin, HIGH);
}

// Получени и синхронизация получения

int LedIntr::receiveInt() {
    if (_rxPin == -1) return 0;

    int analogVal = analogRead(_rxPin); 

    _adapter->delay(_bitDuration);
    return analogVal;
}

uint8_t LedIntr::receiveBit() {
    if (_rxPin == -1) return 0;

    int analogVal = analogRead(_rxPin);
    uint8_t bit = (analogVal > _threshold) ? 1 : 0;

    _adapter->delay(_bitDuration);
    return bit;
}

uint8_t LedIntr::receiveBitNonBlocking() {
    if (_rxPin == -1) return false;

    int analogVal = analogRead(_rxPin);
    uint8_t bit = (analogVal > _threshold) ? 1 : 0;

    return bit;
}

bool LedIntr::checkThreeOnes() {
    if (_rxPin == -1) return false;

    for (int i = 0; i < 3; i++) {
        if (!receiveBitNonBlocking()) {
            return false;
        }
        _adapter->delay(_bitDuration);
    }
    return true;
}

bool LedIntr::findGreetMes(unsigned long timeoutMs) {
    unsigned long startTime = millis();

    while (millis() - startTime < timeoutMs) {
        if (checkThreeOnes()) {
            // Ждём старт-бит 0
            while (receiveBitNonBlocking() != 0) {
                if (millis() - startTime >= timeoutMs) return false;
            }
            _adapter->delay(_bitDuration); // задержка после старт-бита
            return true;
        }
    }
    return false; // таймаут
}

bool LedIntr::receivePacket(uint8_t* buffer, int maxLength) {
    if (!findGreetMes()) return false;

    memset(buffer, 0, maxLength); // обнуляем буфер

    int onesCount = 0;
    int byteIdx = 0;
    int bitPos = 7;

    while (byteIdx < maxLength) {
        bool bit = receiveBit();

        if (bit) {
            onesCount++;
            buffer[byteIdx] |= (1 << bitPos);

            if (onesCount == 5) {
                // стаффинг
                bool nextBit = receiveBit();
                _adapter->delay(_bitDuration); // задержка после стаффинга
                if (nextBit == 0) {
                    onesCount = 0; // игнорируем стаффинг
                } else {
                    break; // конец пакета или ошибка
                }
            }
        } else {
            buffer[byteIdx] &= ~(1 << bitPos);
            onesCount = 0;
        }

        bitPos--;
        if (bitPos < 0) {
            bitPos = 7;
            byteIdx++;
            if (byteIdx >= maxLength) break; // проверка переполнения буфера
        }
    }

    return byteIdx > 0;
}

// void LedIntr::send(const String& text) {
//     for (size_t i = 0; i < text.length(); i++) {
//         uint8_t byteVal = (uint8_t)text[i];

//         Serial.print("Отправляем символ: ");
//         Serial.println(text[i]);
//         Serial.print("Биты: ");

//         // стартовый бит (0)
//         sendBit(0);
//         delay(_bitDuration);

//         // MSB
//         for (int bit = 7; bit >= 0; bit--) {
//             uint8_t b = (byteVal >> bit) & 1;
//             sendBit(b);
//             Serial.print(b);
//         }

//         // стоповый бит (1)
//         sendBit(1);
//         Serial.println("1 [STOP]");
//         delay(_bitDuration * 2);
//     }
// }

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

//     _adapter->delay(_bitDuration);
//     return bit;
// }

// void LedIntr::receiveLoop(Stream& output) {
//     if (_rxPin == -1) return;
    
//     while (true) {
//         while (receiveBit() != 0) {}

//         uint8_t byteVal = 0;
//         for (int bit = 0; bit < 8; bit++) {
//             uint8_t b = receiveBit();
//             byteVal |= (b & 1) << bit;
//         }

//         if (receiveBit() == 1) {
//             char c = (char)byteVal;
//             output.print(c);
//         }
//     }
// }

// void LedIntr::findGreetMes() {
//     if (_rxPin == -1) return;

//     int consecutiveOnes = 0;

//     while (true) {
//         int bit = receiveBit();

//         if (bit == 1) {
//             consecutiveOnes++;
//             if (consecutiveOnes == 3) {
//                 // ждём, пока придёт 0 — стартовый бит
//                 while (receiveBit() != 0) {}
//                 // сигнал найден, выходим
//                 break;
//             }
//         } else {
//             // сбрасываем счётчик, если встретили 0 до трёх единиц
//             consecutiveOnes = 0;
//         }
//     }
// }
