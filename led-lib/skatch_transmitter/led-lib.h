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
    virtual ~IGpioAdapter() {}
};

// Кодировщик / декодировщик
class LedIntr {
public:
    LedIntr(IGpioAdapter* adapter, int txPin = -1, int rxPin = -1, int bitDuration = 500);

    void sendBit(uint8_t bit);
    void sendBitNonBlocking(uint8_t bit);
    void sendPacket(const uint8_t* data, int length);
    void sendGreetMes();

    int receiveInt();
    uint8_t receiveBit();
    uint8_t receiveBitNonBlocking();
    bool receivePacket(uint8_t* buffer, int maxLength);
    bool findGreetMes(unsigned long timeoutMs = 5000);

    bool checkThreeOnes();
    void setBitDuration(unsigned long bitDuration);
    unsigned long bitDuration() const { return _bitDuration; }

    void setThreshold(uint16_t threshold);
    uint16_t threshold() const { return _threshold; }

    bool receiveMessage(String& out, size_t maxChars = 64, unsigned long startTimeoutMs = 5000);
    void receiveLoop(Stream& output = Serial);

private:
    IGpioAdapter* _adapter;
    int _txPin;
    int _rxPin;
    int _bitDuration;

    uint16_t _threshold;

    bool receiveByte(uint8_t& byteOut, unsigned long startTimeoutMs);
};

#endif
