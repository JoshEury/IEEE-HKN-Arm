#pragma once

#include <Arduino.h>

/*
*   Parametric machine code generators for a subset of the AVR instruction set
*/

// Instruction word type
typedef uint16_t iword;

// Enumerate the flags in the status register
enum class Flags : byte {C, Z, N, V, S, H, T, I};

// Use an unallocated instruction as a terminator for blocks of bytecode
const iword END_BLOCK = 0x0001;

// Return from a subroutine
const iword RET = 0x9508;

// Push a register onto the stack
constexpr iword PUSH(const byte reg) {
    return 0x920F | (reg << 4);
}

// Pop a register off of the stack
constexpr iword POP(const byte reg) {
    return 0x900F | (reg << 4);
}

// Load a byte from an IO location into a register
constexpr iword IN(const byte reg, const byte src) {
    return 0xB000 | ((src & 0x30) << 5) | (reg << 4) | (src & 0xF);
}

// Store a byte from a register to an IO location
constexpr iword OUT(const byte reg, const byte src) {
    return 0xB800 | ((src & 0x30) << 5) | (reg << 4) | (src & 0xF);
}

// Load a byte from memory into a register (memory address pointed to by Z register)
constexpr iword LDZ(const byte reg) {
    return 0x8000 | (reg << 4);
}

// Store a byte from a register to a memory location (memory address pointed to by Z register)
constexpr iword STZ (const byte reg) {
    return 0x8200 | (reg << 4);
}

// Load an immediate value into a register
constexpr iword LDI(const byte reg, const byte imm) {
    return 0xE000 | ((reg & 0xF) << 4) | ((imm & 0xF0) << 4) | (imm & 0xF);
}

// Perform a bitwise AND of two registers and store the result in Rd
constexpr iword AND(const byte Rd, const byte Rr) {
    return 0x2000 | ((Rr & 0x10) << 5) | (Rd << 4) | (Rr & 0xF);
}

// Perform an exclusive OR of two registers and store the result in Rd
constexpr iword EOR(const byte Rd, const byte Rr) {
    return 0x2400 | ((Rr & 0x10) << 5) | (Rd << 4) | (Rr & 0xF);
}

// Perform a bitwise OR of two registers and store the result in Rd
constexpr iword OR(const byte Rd, const byte Rr) {
    return 0x2800 | ((Rr & 0x10) << 5) | (Rd << 4) | (Rr & 0xF);
}

// Compare a register with an immediate value
constexpr iword CPI(const byte reg, const byte imm) {
    return 0x3000 | ((imm & 0xF0) << 4) | ((reg & 15) << 4) | (imm & 0xF);
}

// Branch if a specified bit in the status register is set
constexpr iword BRBS(const Flags bit, const int8_t distance) {
    return 0xF000 | ((distance & 0x7F) << 3) | ((byte)bit & 0x7);
}

// Branch if a specified bit in the status register is cleared
constexpr iword BRBC(const Flags bit, const int8_t distance) {
    return 0xF400 | ((distance & 0x7F) << 3) | ((byte)bit & 0x7);
}

// Unconditional relative jump
constexpr iword JMP(const int16_t distance) {
    return 0xC000 | (distance & 0x0FFF);
}
