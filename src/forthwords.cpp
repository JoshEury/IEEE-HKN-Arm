#include <Arduino.h>
#include <ArduinoSTL.h>
#include "forthwords.h"

// Create a stack of addresses
AddrStack::AddrStack() : stack{}, stackPtr(stack) {}
AddrStack::~AddrStack() {
    // delete[] stack;
}

// Push an element onto an address stack
void AddrStack::push(uintptr_t address) {
    *stackPtr++ = address;
}

// Pop an element off of an address stack
uintptr_t AddrStack::pop() {
    return *--stackPtr;
}

// Create an address stack to hold the buffer addresses for retroactive branch insertion
AddrStack branches;

// Generate bytecode for an IF word
void generateIf(byte*& buffer) {
    for (iword instr : {POP(30), CPI(30, 0)}) {
        *buffer++ = instr & 0xFF;
        *buffer++ = instr >> 8;
    }
    // Defer branch opcode generation until branch destination is known
    branches.push((uintptr_t)buffer);
    // Leave space for the branch opcode to be retroactively inserted
    buffer += 2;
}

// Generate bytecode for an ELSE word
void generateElse(byte*& buffer) {
    // Insert conditional branch at the location of the IF statement
    byte* instrLoc = (byte*)branches.pop();
    iword branchCode = BRBS(Flags::Z, uintptr_t(buffer - instrLoc) >> 1);   // Jump past the unconditional jump that begins the ELSE predicate
    *instrLoc++ = branchCode & 0xFF;
    *instrLoc = branchCode >> 8;
    // If the IF predicate executes, it needs to skip the ELSE predicate
    branches.push((uintptr_t)buffer);
    // Leave space for the jump opcode to be retroactively inserted
    buffer += 2;
}

// Generate bytecode for a THEN word
void generateThen(byte*& buffer) {
    // Insert unconditional branch at the location of the ELSE statement
    // Fix later - this implementation necessitates an ELSE or the IF will jump unconditionally
    byte* instrLoc = (byte*)branches.pop();
    iword rjmpCode = JMP((uintptr_t(buffer - instrLoc) >> 1) - 1);
    *instrLoc++ = rjmpCode & 0xFF;
    *instrLoc = rjmpCode >> 8;
    // Note that THEN does not generate any machine code of its own
}