#ifndef PLATFORM_LIB_H
#define PLATFORM_LIB_H

#include <Arduino.h>
#include "led-lib.h"

// supports both ESP (via Arduino core only!) and Arduino
class GenericAdapter : public IGpioAdapter {
public:
    void pinWrite(int pin, bool value) override {
        digitalWrite(pin, value ? HIGH : LOW);
    }

    bool pinRead(int pin) override {
        return digitalRead(pin) == HIGH;
    }

    void delay(unsigned long ms) override {
        ::delay(ms);
    }

    void delayMicro(unsigned long us) override {
        delayMicroseconds(us);
    }
};

#endif
