#ifndef LEDINTR_H
#define LEDINTR_H

#include <Arduino.h>

// Универсальный интерфейс GPIO
class IGpioAdapter {
public:
    virtual void pinWrite(int pin, bool value) = 0;
    virtual bool pinRead(int pin) = 0;
    virtual void delay(unsigned long ms) = 0;
    virtual void delayMicro(unsigned long us) = 0;
    virtual ~IGpioAdapter() {} // Добавляем виртуальный деструктор
};

// Кодировщик / декодировщик
class LedIntr {
public:
    LedIntr(IGpioAdapter* adapter, int txPin = -1, int rxPin = -1, unsigned long bitDelayUs = 500);

    void send(const String& text);
    void receiveLoop();

private:
    IGpioAdapter* _adapter;
    int _txPin;
    int _rxPin;
    unsigned long _bitDelay;
    int _threshold = 500;

    void sendBit(uint8_t bit);
    uint8_t receiveBit();
};

#endif
