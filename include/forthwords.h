#pragma once

#include <Arduino.h>
#include <ArduinoSTL.h>
#include "opcodes.h"

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

// A pointer to a void function with a byte pointer parameter
typedef void (*iwordGenerator)(byte*&);

// Convenience template function to generate bytecode for words with deterministic bytecode
template <iword... iwords>
void makeWord(byte*& buffer) {
    for (iword instr : {iwords...}) {
        *buffer++ = instr & 0xFF;
        *buffer++ = instr >> 8;
    }
}

const char names[] PROGMEM = {
    "LOAD\0"    \
    "STORE\0"   \
    "AND\0"     \
    "OR\0"      \
    "XOR\0"     \
    "IF\0"      \
    "ELSE\0"    \
    "THEN\0"
};

void generateIf(byte*& buffer);
void generateElse(byte*& buffer);
void generateThen(byte*& buffer);

// Lookup table of pointers to functions that generate bytecode for each word
const iwordGenerator generators[] PROGMEM = {
    makeWord<POP(30), POP(31), LDZ(0), PUSH(0)>,        // LOAD
    makeWord<POP(30), POP(31), POP(0), STZ(0)>,         // STORE
    makeWord<POP(30), POP(0), AND(30, 0), PUSH(30)>,    // AND
    makeWord<POP(30), POP(0), OR(30, 0), PUSH(30)>,     // OR
    makeWord<POP(30), POP(0), EOR(30, 0), PUSH(30)>,    // XOR
    generateIf,
    generateElse,
    generateThen
};