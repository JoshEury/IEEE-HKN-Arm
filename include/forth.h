#pragma once

#include <Arduino.h>
#include "opcodes.h"

class Lexer {
    char* input;

public:
    Lexer(char* inputStr);
    void setStart(char* inputStr);
    char* next();
};

class AddrStack {
    uintptr_t stack[8];
    uintptr_t* stackPtr;

public:
    AddrStack();
    ~AddrStack();
    AddrStack& operator = (AddrStack& cpy) = delete;
    void push(uintptr_t address);
    uintptr_t pop();
};

void compile(Lexer& tokens, byte* buffer);

const iword* translate(const char* token);

// Perform a call to a given memory address
inline void call(const byte *ptr) {
    asm volatile(
        "lsr %B0              \n\t"   // Right-shift high Z register
        "ror %A0              \n\t"   // Rotate carry into low Z register
        "icall                \n\t"
        : : "z" (ptr) : "r0", "memory"
    );
}