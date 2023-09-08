#pragma once

#include <Arduino.h>

class A4988 {
    byte stepPin;
    byte dirPin;

public:
    A4988(byte stepPin, byte dirPin);
};