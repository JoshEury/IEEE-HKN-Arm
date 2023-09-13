#pragma once

#include <Arduino.h>
#include "opcodes.h"

const char names[] PROGMEM = {
    "LOAD\0"    \
    "STORE\0"   \
    "AND\0"     \
    "OR\0"      \
    "XOR\0"
};

const iword bytecodes[] PROGMEM = {
// LOAD
    POP(30), POP(31), LDZ(0), PUSH(0), END_BLOCK,       // Pop (ZL, ZH) -> Push *Z
// STORE
    POP(30), POP(31), POP(0), STZ(0), END_BLOCK,        // Pop (ZL, ZH, *Z) -> *Z stored to Z
// AND
    POP(30), POP(0), AND(30, 0), PUSH(30), END_BLOCK,   // Pop (A, B) -> Push A AND B
// OR
    POP(30), POP(0), OR(30, 0), PUSH(30), END_BLOCK,    // Pop (A, B) -> Push A OR B
// XOR
    POP(30), POP(0), EOR(30, 0), PUSH(30), END_BLOCK    // Pop (A, B) -> Push A XOR B
};