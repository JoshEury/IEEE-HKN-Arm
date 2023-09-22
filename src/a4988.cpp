#include <Arduino.h>
#include "a4988.h"
#include "hal.h"

A4988* A4988::instanceOCR2A = nullptr;
A4988* A4988::instanceOCR2B = nullptr;

// Initialize the hardware timer
void A4988::init() {
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    TCCR2A = bit(WGM21);
    TCCR2B = 0;

    (stepPin == 11 ? A4988::instanceOCR2A : A4988::instanceOCR2B) = this;
    noInterrupts();
    TIMSK2 |= bit(stepPin == 11 ? OCIE2A : OCIE2B);
    interrupts();
}

void A4988::setSpeed(byte speed) {
    if (speed) {
        TCCR2A |= bit(stepPin == 11 ? COM2A0 : COM2B0);     // Connect timer to output pin
        (stepPin == 11 ? OCR2A : OCR2B) = speed;
        TCCR2B |= bit(CS20) | bit(CS21) | bit(CS22);        // Timer prescaler 1024
    }
    else {
        TCCR2A &= ~bit(stepPin == 11 ? COM2A0 : COM2B0);    // Disconnect timer from output pin
        TCCR2B &= ~(bit(CS20) | bit(CS21) | bit(CS22));     // Timer stopped
        digitalWrite(stepPin, LOW);
    }
}

void TIMER2_COMPA_vect(void) {
    if (A4988::instanceOCR2A) { // Is there an A4988 instance associated with this pin?
        A4988::instanceOCR2A->currentPos += PORT(A4988::instanceOCR2A->stepPin) ? 1 : -1;
    }
    else {
        TIMSK2 &= ~bit(OCIE2A); // Disable this interrupt
    }
}

void TIMER2_COMPB_vect(void) {
    if (A4988::instanceOCR2B) { // Is there an A4988 instance associated with this pin?
        A4988::instanceOCR2B->currentPos += PORT(A4988::instanceOCR2B->stepPin) ? 1 : -1;
    }
    else {
        TIMSK2 &= ~bit(OCIE2B); // Disable this interrupt
    }
}