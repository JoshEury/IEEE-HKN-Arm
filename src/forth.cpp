#include <ArduinoSTL.h>
#include "forth.h"
#include "forthwords.h"

Lexer::Lexer(char* inputStr) : input(inputStr)
{}

// Assign a new input string to the lexer
void Lexer::setStart(char* inputStr) {
    input = inputStr;
}

// Fetch the next token in the input string
char* Lexer::next() {
    char* chrAt = input;
    while (*chrAt && *chrAt != ' ' && *chrAt != '\n') ++chrAt;
    if (*chrAt) {
        *chrAt = '\0';      // Replace the whitespace with a null terminator
        char* start = input;
        input = chrAt + 1;  // Move the lexer input pointer to the start of the next token
        return start;       // Return a pointer to the start of the lexed token
    }
    else if (*input) {
        char* start = input;
        input = chrAt;
        return start;
    }
    else return nullptr;
}

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

// Compile a token stream into bytecode stored in a page buffer
void compile(Lexer& tokens, byte* buffer) {
    char* token;
    AddrStack branches;
    while ((token = tokens.next())) {
        const iword* instrString = translate(token);
        if (instrString)
            while (pgm_read_word(instrString) != END_BLOCK) {
                // AVR instructions are stored little-endian
                *buffer++ = pgm_read_word(instrString) & 0xFF;
                *buffer++ = pgm_read_word(instrString++) >> 8;
            }
        else {
            byte number = 0;
            if (!strcmp_P(token, (const char*)F("0")) || (number = atoi(token))) {
                for (iword instr : {LDI(30, number), PUSH(30)}) {
                    *buffer++ = instr & 0xFF;
                    *buffer++ = instr >> 8;
                }
            }
            else if (!strcmp_P(token, (const char*)F("IF"))) {
                for (iword instr : {POP(30), CPI(30, 0)}) {
                    *buffer++ = instr & 0xFF;
                    *buffer++ = instr >> 8;
                }
                // Defer branch opcode generation until branch destination is known
                branches.push((uintptr_t)buffer);
                // Leave space for the branch opcode to be retroactively inserted
                buffer += 2;
            }
            else if (!strcmp_P(token, (const char*)F("ELSE"))) {
                // Insert conditional branch at the location of the IF statement
                byte* instrLoc = (byte*)branches.pop();
                iword branchCode = BRBS(Flags::Z, uintptr_t(buffer - instrLoc) >> 1);
                *instrLoc++ = branchCode & 0xFF;
                *instrLoc = branchCode >> 8;
                // If the IF predicate executes, it needs to skip the ELSE predicate
                branches.push((uintptr_t)buffer);
                // Leave space for the jump opcode to be retroactively inserted
                buffer += 2;
            }
            else if (!strcmp_P(token, (const char*)F("THEN"))) {
                // Insert unconditional branch at the location of the ELSE statement
                // Fix later - this implementation necessitates an ELSE or the IF will jump unconditionally
                byte* instrLoc = (byte*)branches.pop();
                iword rjmpCode = JMP((uintptr_t(buffer - instrLoc) >> 1) - 1);
                *instrLoc++ = rjmpCode & 0xFF;
                *instrLoc = rjmpCode >> 8;
                // Note that THEN does not generate any machine code of its own
            }
        }
    }

    // Add a return instruction at the end of the FORTH program
    *buffer++ = RET & 0xFF;
    *buffer = RET >> 8;
}

// Fetch the instruction word sequence associated with the given token
const iword* translate(const char* token) {
    const char* knownWord = names;
    uint16_t wordIdx = 0;
    while (strcmp_P("", knownWord)) {
        if (!strcmp_P(token, knownWord)) {
            const iword* bytecodePtr = bytecodes;
            uint16_t bytecodeIdx = 0;
            while (bytecodeIdx < wordIdx) {
                // Advance the bytecode pointer to the start of the next word's bytecode
                while (pgm_read_word(bytecodePtr) != END_BLOCK) ++bytecodePtr;
                ++bytecodePtr;

                ++bytecodeIdx;
            }
            return bytecodePtr;
        }
        ++wordIdx;
        
        // Advance the knownWord pointer to the start of the next known word
        while (pgm_read_byte(knownWord)) ++knownWord;
        ++knownWord;
    }
    return nullptr;
}