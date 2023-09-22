#pragma once

#include <Arduino.h>

extern "C" {
    void TIMER2_COMPA_vect(void) __attribute__((signal, __INTR_ATTRS));
    void TIMER2_COMPB_vect(void) __attribute__((signal, __INTR_ATTRS));
}

class A4988 {
    const byte stepPin;
    const byte dirPin;
    int16_t currentPos;
    int16_t targetPos;
    float currentSpeed;
    static A4988* instanceOCR2A;
    static A4988* instanceOCR2B;
    friend void ::TIMER2_COMPA_vect(void);
    friend void ::TIMER2_COMPB_vect(void);

public:
    constexpr A4988(const byte stepPin, const byte dirPin);
    void init();
    void setSpeed(byte speed);
};

constexpr A4988::A4988(const byte stepPin, const byte dirPin) 
    : stepPin(stepPin)
    , dirPin(dirPin)
    , currentPos(0)
    , targetPos(0)
{}