#include "led-lib.h"
#include <Arduino.h>

namespace {
inline unsigned long toMs(unsigned long us) {
    return (us + 999UL) / 1000UL;
}
}

constexpr uint16_t LedIntr::analogHighLevel() {
#if defined(ARDUINO_ARCH_ESP32)
    return 4095;
#elif defined(ARDUINO_ARCH_AVR)
    return 1023;
#else
    return 4095;
#endif
}

LedIntr::LedIntr(IGpioAdapter* adapter, int txPin, int rxPin, unsigned long bitDelayUs)
    : _adapter(adapter),
      _txPin(txPin),
      _rxPin(rxPin),
      _bitDurationUs(bitDelayUs ? bitDelayUs : 1000UL),
      _threshold(analogHighLevel() / 2),
      _samplesPerBit(3),
      _halfBitUs(1) {
    updateTiming();

    if (_txPin != -1) {
        pinMode(_txPin, OUTPUT);
        if (_adapter) {
            _adapter->pinWrite(_txPin, HIGH);
        } else {
            digitalWrite(_txPin, HIGH);
        }
    }

    if (_rxPin != -1) {
        pinMode(_rxPin, INPUT);
    }
}

void LedIntr::sendBit(uint8_t bit) {
    if (_txPin == -1 || !_adapter) {
        return;
    }

    _adapter->pinWrite(_txPin, bit ? HIGH : LOW);
    waitInterval(_bitDurationUs);
}

void LedIntr::sendByte(uint8_t value) {
    if (_txPin == -1 || !_adapter) {
        return;
    }

    sendBit(0);  // стартовый бит

    for (uint8_t bit = 0; bit < 8; ++bit) {
        sendBit((value >> bit) & 0x1);
    }

    sendBit(1);  // стоповый бит
}

void LedIntr::send(const String& text) {
    if (_txPin == -1 || !_adapter) {
        return;
    }

    for (size_t i = 0; i < text.length(); ++i) {
        sendByte(static_cast<uint8_t>(text[i]));
    }
}

void LedIntr::setBitDuration(unsigned long bitDurationUs) {
    if (bitDurationUs == 0) {
        return;
    }

    _bitDurationUs = bitDurationUs;
    updateTiming();
}

void LedIntr::setThreshold(uint16_t threshold) {
    _threshold = threshold;
}

void LedIntr::setSamplesPerBit(size_t samples) {
    _samplesPerBit = samples == 0 ? 1 : samples;
}

void LedIntr::autoCalibrate(size_t sampleCount, unsigned long sampleDelayUs) {
    if (_rxPin == -1 || sampleCount == 0) {
        return;
    }

    uint16_t minVal = analogHighLevel();
    uint16_t maxVal = 0;

    for (size_t i = 0; i < sampleCount; ++i) {
        uint16_t val = sampleAnalog();
        if (val < minVal) {
            minVal = val;
        }
        if (val > maxVal) {
            maxVal = val;
        }

        if (sampleDelayUs) {
            waitInterval(sampleDelayUs);
        }
    }

    if (maxVal > minVal) {
        _threshold = minVal + ((maxVal - minVal) / 2);
    }
}

bool LedIntr::receiveMessage(String& out, size_t maxChars, unsigned long startTimeoutMs) {
    out = "";
    if (_rxPin == -1 || maxChars == 0) {
        return false;
    }

    uint8_t byteVal = 0;
    if (!receiveByte(byteVal, startTimeoutMs)) {
        return false;
    }

    out.reserve(maxChars);
    out += static_cast<char>(byteVal);

    const unsigned long frameTimeoutMs = toMs(_bitDurationUs * 12UL) + 1UL;

    while (out.length() < maxChars) {
        if (!receiveByte(byteVal, frameTimeoutMs)) {
            break;
        }

        out += static_cast<char>(byteVal);
        if (byteVal == '\n' || byteVal == '\0') {
            break;
        }
    }

    return true;
}

void LedIntr::receiveLoop(Stream& output) {
    if (_rxPin == -1) {
        return;
    }

    while (true) {
        uint8_t value = 0;
        if (receiveByte(value, 0)) {
            output.write(value);
        } else {
            waitInterval(_bitDurationUs);
        }
        yield();
    }
}

uint8_t LedIntr::receiveBit() {
    if (_rxPin == -1) {
        return 0;
    }

    const size_t samples = _samplesPerBit == 0 ? 1 : _samplesPerBit;
    uint32_t sum = 0;
    for (size_t i = 0; i < samples; ++i) {
        sum += sampleAnalog();
    }

    const uint16_t avg = static_cast<uint16_t>(sum / samples);
    return (avg > _threshold) ? 1 : 0;
}

int LedIntr::receiveInt() {
    return static_cast<int>(sampleAnalog());
}

uint16_t LedIntr::sampleAnalog() {
    if (_rxPin == -1) {
        return 0;
    }

#if defined(ARDUINO_ARCH_AVR)
    if (_rxPin >= A0) {
        return static_cast<uint16_t>(analogRead(_rxPin));
    }
    const bool highLevel = _adapter ? _adapter->pinRead(_rxPin) : (digitalRead(_rxPin) == HIGH);
    return highLevel ? analogHighLevel() : 0;
#else
    return static_cast<uint16_t>(analogRead(_rxPin));
#endif
}

void LedIntr::waitInterval(unsigned long durationUs) {
    if (durationUs == 0) {
        return;
    }

    if (_adapter) {
        _adapter->delayMicro(durationUs);
    } else {
        delayMicroseconds(durationUs);
    }
}

void LedIntr::updateTiming() {
    _halfBitUs = _bitDurationUs / 2UL;
    if (_halfBitUs == 0) {
        _halfBitUs = 1;
    }
}

bool LedIntr::waitForStartBit(unsigned long timeoutMs) {
    if (_rxPin == -1) {
        return false;
    }

    const bool finiteTimeout = timeoutMs != 0;
    const unsigned long start = millis();

    while (true) {
        if (receiveBit() == 0) {
            waitInterval(_halfBitUs);
            if (receiveBit() == 0) {
                return true;
            }
        }

        if (finiteTimeout && (millis() - start >= timeoutMs)) {
            return false;
        }

        yield();
    }
}

bool LedIntr::receiveByte(uint8_t& byteOut, unsigned long startTimeoutMs) {
    if (!waitForStartBit(startTimeoutMs)) {
        return false;
    }

    uint8_t value = 0;
    for (uint8_t bit = 0; bit < 8; ++bit) {
        waitInterval(_bitDurationUs);
        const uint8_t sample = receiveBit();
        value |= (sample & 0x1) << bit;
    }

    waitInterval(_bitDurationUs);
    if (receiveBit() == 0) {
        return false;
    }

    byteOut = value;
    return true;
}
