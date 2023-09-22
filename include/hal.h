#pragma once

#include <Arduino.h>

// Define a type for IO register addresses
typedef decltype(PORTB) Register;

// Make a nice interface for register manipulation of IO ports
class MaskedRegister {
private:
    Register address;    // IO register address
    const byte mask;     // Bit mask for target pin

public:
    constexpr MaskedRegister(const Register address, const byte pin)
        : address(address)
        , mask(1 << (pin < 14 ? pin & 7 : (pin+2) & 7))   // Translate a pin number to its associated bit in hardware IO registers
    {}
    constexpr explicit operator Register () {   // Return a reference to the hardware register associated with the target pin
        return address;
    }
    operator bool () {                          // Return the value of the target bit
        return address & mask;
    }
    MaskedRegister& operator = (MaskedRegister& reg) {  // Set the target bit equal to the value of another MaskedRegister's target bit
        *this = (bool)reg;
        return *this;
    }
    byte operator = (const bool rhs) {          // Set the target bit equal to the right hand side
        return address = rhs ? address | mask : address & ~mask;
    }
    bool operator &= (const bool rhs) {
        return *this = *this & rhs;
    }
    bool operator ^= (const bool rhs) {
        return *this = *this ^ rhs;
    }
    bool operator |= (const bool rhs) {
        return *this = *this | rhs;
    }
};

// Get the port-based REGx register associated with a given pin
#define PIN_REGISTER_LOOKUP(registerName) \
constexpr MaskedRegister registerName(const byte pin) { \
    return MaskedRegister(\
        pin < 8 ?   registerName##D : \
        pin < 14 ?  registerName##B : \
        registerName##C\
    , pin); \
}

// Create lookups for IO registers
PIN_REGISTER_LOOKUP(PORT)
PIN_REGISTER_LOOKUP(DDR)
PIN_REGISTER_LOOKUP(PIN)