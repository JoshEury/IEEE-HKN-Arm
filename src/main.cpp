#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>
#include "optiboot.h"
#include "a4988.h"
#include "forth.h"

// #define DEBUG

const byte BASE_STEP_PIN = 3;
const byte BASE_DIR_PIN = 2;
const byte ELBOW_A_PIN = 9;
const byte ELBOW_B_PIN = 10;

// A page of flash memory that initially contains only the machine code for RET
const byte forthPgm[SPM_PAGESIZE] __attribute__((aligned(SPM_PAGESIZE))) PROGMEM = {
    RET & 255, RET >> 8  // RET
};

// A buffer to hold a page of memory while writing to flash
byte pgBuffer[SPM_PAGESIZE];

// Servos are on pins 9 and 10
Servo elbowA;
Servo elbowB;

// Stepper motor (step pin 3, direction pin 2)
A4988 base(BASE_STEP_PIN, BASE_DIR_PIN);

// Input stream of FORTH tokens
// char iStream[] = "0 37 LOAD 32 XOR 0 37 STORE";  // Toggle pin 13
char iStream[] = "0 37 LOAD 0 35 LOAD 16 AND IF 223 AND ELSE 32 OR THEN 0 37 STORE"; // Set D13 to !D12
Lexer lexer(iStream);

// Dump a section of flash memory to the serial console
void dump(const byte* ptr, uint16_t len) {
    Serial.printf(F("Dump of %04X:\n"), ptr);
    for (uint16_t i = 0; i < len; i++) {
        if ((i&15) == 0) Serial.println();
        Serial.printf(F("%02X "), pgm_read_byte((uint16_t)ptr + i));
    }
    Serial.println();
}

void setup() {
    // put your setup code here, to run once:
    elbowA.attach(ELBOW_A_PIN);
    elbowB.attach(ELBOW_B_PIN);

    pinMode(13, OUTPUT);

    Serial.begin(115200);
    Serial.println("Compiling FORTH...");
    compile(lexer, pgBuffer);

#ifdef DEBUG
    for (int i=0; i<SPM_PAGESIZE; ++i) {
        if (pgBuffer[i] < 0x10) Serial.print('0');
        Serial.print(pgBuffer[i], 16);
        if ((i & 15) == 15) Serial.println();
        else Serial.print(' ');
    }

    Serial.println();
    Serial.flush();

    dump(forthPgm, 128);
    Serial.flush();

    delay(1000);
#endif  // DEBUG

    if (EEPROM.read(0)) {
        Serial.println(F("Modifying flash contents..."));
        optiboot_writePage(forthPgm, pgBuffer, 0);
        Serial.println(F("New flash written!"));
    }
    else {
        Serial.println(F("EEPROM 0x00 is 0 - flash already written."));
    }
    EEPROM.update(0, 0);

    Serial.println(F("Executing..."));
    delay(1000);
}

void loop() {
    // put your main code here, to run repeatedly:
    unsigned long start = micros();
    call(forthPgm);
    unsigned long finish = micros();
    Serial.print("Array call took ");
    Serial.print(finish - start);
    Serial.println(" microseconds");
    delay(500);
}

void serialEvent() {
    while (Serial.available()) {
        if ((char)Serial.read() == 'C') {
            EEPROM.update(0, 255);
            Serial.println(F("EEPROM cleared! Reflashing code will be active on next reset."));
        }
    }
    if (!EEPROM.read(0)) Serial.println(F("Enter 'C' to clear EEPROM reflash guard."));
}