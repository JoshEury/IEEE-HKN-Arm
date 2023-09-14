#pragma once

#include <Arduino.h>
#include "forthwords.h"
#include "opcodes.h"

class Lexer {
    char* input;

public:
    Lexer(char* inputStr);
    void setStart(char* inputStr);
    char* next();
};

void compile(Lexer& tokens, byte* buffer);

const iwordGenerator translate(const char* token);

// Perform a call to a given memory address
inline void call(const byte *ptr) {
    asm volatile(
        "lsr %B0              \n\t"   // Right-shift high Z register
        "ror %A0              \n\t"   // Rotate carry into low Z register
        "icall                \n\t"
        : : "z" (ptr) : "r0", "memory"
    );
}