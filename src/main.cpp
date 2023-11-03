#include <Arduino.h>
#include <NeoHWSerial.h>
#include <Servo.h>
#include <EEPROM.h>
// #include <AccelStepper.h>
#include <ArduinoSTL.h>
#include "optiboot.h"
#include "A4988.h"
#include "forth.h"

// #define DEBUG
// :)

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
// AccelStepper base(AccelStepper::DRIVER, BASE_STEP_PIN, BASE_DIR_PIN);
// A4988 base(BASE_STEP_PIN, BASE_DIR_PIN);
A4988 base(200, BASE_DIR_PIN, BASE_STEP_PIN);

// Input stream of FORTH tokens
// char iStream[] = "0 37 LOAD 32 XOR 0 37 STORE";  // Toggle pin 13
// char iStream[] = "0 37 LOAD 0 35 LOAD 16 AND IF 223 AND ELSE 32 OR THEN 0 37 STORE"; // Set D13 to !D12
char iStream[1024] = "";
uint16_t iStreamLen = 0;
Lexer lexer(iStream);

// Dump a section of flash memory to the serial console
void dump(const byte* ptr, uint16_t len) {
    NeoSerial.printf(F("Dump of %04X:\n"), ptr);
    for (uint16_t i = 0; i < len; i++) {
        if ((i&15) == 0) NeoSerial.println();
        NeoSerial.printf(F("%02X "), pgm_read_byte((uint16_t)ptr + i));
    }
    NeoSerial.println();
}

void clearIStream() {
    uint16_t i = 0;
    while (iStream[i]) iStream[i++] = '\0';
    iStreamLen = 0;
}

static void SerialEvent(uint8_t chr);
volatile bool lineReady = false;
bool prgMode = true;

int16_t target = 1000;

void setup() {
    // put your setup code here, to run once:
    elbowA.attach(ELBOW_A_PIN);
    elbowB.attach(ELBOW_B_PIN);

    pinMode(13, OUTPUT);

    // base.setSpeed(200);
    // base.setAcceleration(100);
    // pinMode(BASE_DIR_PIN, OUTPUT);
    // pinMode(BASE_STEP_PIN, OUTPUT);
    // digitalWrite(BASE_DIR_PIN, HIGH);
    base.begin(60, 1);
    base.setSpeedProfile(base.LINEAR_SPEED, 1000, 1000);

    NeoSerial.attachInterrupt(SerialEvent);
    NeoSerial.begin(115200);
    // NeoSerial.println(F("Compiling FORTH..."));
    // compile(lexer, pgBuffer);

#ifdef DEBUG
    dump(forthPgm, 128);
    NeoSerial.flush();

    delay(1000);
#endif  // DEBUG

    // if (EEPROM.read(0)) {
    //     NeoSerial.println(F("Modifying flash contents..."));
    //     optiboot_writePage(forthPgm, pgBuffer, 0);
    //     NeoSerial.println(F("New flash written!"));
    // }
    // else {
    //     NeoSerial.println(F("EEPROM 0x00 is 0 - flash already written."));
    // }
    // EEPROM.update(0, 0);

    NeoSerial.println(F("Welcome to interactive FORTH!"));
    delay(1000);
}

void loop() {
    if (lineReady) {
        lineReady = false;
        NeoSerial.flush();

        if (prgMode) {
            if (!strncmp(iStream, "exit", 4)) {
                NeoSerial.println("\nLeaving FORTH");
                prgMode = false;
                clearIStream();
                return;
            }

            // Echo back received program
            NeoSerial.println();
            NeoSerial.println();
            NeoSerial.println(F("Received program:"));
            NeoSerial.println(iStream);

            // Clear page write buffer and reset lexer position
            for (byte i = 0; i < SPM_PAGESIZE; ++i) pgBuffer[i] = 0x00;
            lexer.setStart(iStream);
            
            // Compile program
            NeoSerial.println(F("Compiling FORTH..."));
            compile(lexer, pgBuffer);
            // Write program to flash
            NeoSerial.println(F("Modifying flash contents..."));
            optiboot_writePage(forthPgm, pgBuffer, 0);

            NeoSerial.println(F("New flash written!"));
        }
        else {
            if (!strncmp(iStream, "shell", 5)) {
                prgMode = true;
                clearIStream();
                return;
            }

            // Parse command
            char* token = iStream;
            lexer.setStart(iStream);
            while ((token = lexer.next())) {
                if (*token == 'M') {
                    int16_t moveDist = atoi(++token);
                    base.move(moveDist);
                }
            }
            clearIStream();
        }

#ifdef DEBUG
        for (int i=0; i<SPM_PAGESIZE; ++i) {
            if (pgBuffer[i] < 0x10) NeoSerial.print('0');
            NeoSerial.print(pgBuffer[i], 16);
            if ((i & 15) == 15) NeoSerial.println();
            else NeoSerial.print(' ');
        }

        NeoSerial.println();
        NeoSerial.flush();
#endif

        // Reset receive buffer string and discard any characters received during compilation
        clearIStream();
    }
    // put your main code here, to run repeatedly:
#ifdef DEBUG
    unsigned long start = micros();
#endif
    call(forthPgm);
#ifdef DEBUG
    unsigned long finish = micros();
    NeoSerial.print(F("Array call took "));
    NeoSerial.print(finish - start);
    NeoSerial.println(F(" microseconds"));
#endif
}

static void SerialEvent(uint8_t chr) {
    if (chr == '\n' || chr == '\r') {
        lineReady = true;
    }
    else if (chr == '\b' && iStreamLen) {
        iStream[--iStreamLen] = '\0';
        NeoSerial.print(F("\b \b"));
    }
    else {
        iStream[iStreamLen] = chr;
        iStream[++iStreamLen] = '\0';
        NeoSerial.print((char)chr);
    }
}