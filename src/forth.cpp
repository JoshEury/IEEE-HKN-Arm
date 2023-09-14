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

// Compile a token stream into bytecode stored in a page buffer
void compile(Lexer& tokens, byte* buffer) {
    char* token;
    // AddrStack branches;
    while ((token = tokens.next())) {
        // Look up bytecode generator function pointer using token name
        iwordGenerator generator = translate(token);
        if (generator) generator(buffer);
        else {
            byte number = 0;
            if (!strcmp_P(token, (const char*)F("0")) || (number = atoi(token))) {
                for (iword instr : {LDI(30, number), PUSH(30)}) {
                    *buffer++ = instr & 0xFF;
                    *buffer++ = instr >> 8;
                }
            }
           else Serial.println(F("Error translating token"));
        }
    }

    // Add a return instruction at the end of the FORTH program
    *buffer++ = RET & 0xFF;
    *buffer = RET >> 8;
}

// Fetch the instruction word sequence associated with the given token
const iwordGenerator translate(const char* token) {
    const char* knownWord = names;
    uint16_t wordIdx = 0;
    while (strcmp_P("", knownWord)) {
        if (!strcmp_P(token, knownWord)) {
            return (iwordGenerator) pgm_read_word(generators + wordIdx);
        }
        ++wordIdx;
        
        // Advance the knownWord pointer to the start of the next known word
        while (pgm_read_byte(knownWord)) ++knownWord;
        ++knownWord;
    }
    return nullptr;
}