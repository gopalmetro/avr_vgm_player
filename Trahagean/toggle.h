#ifndef TOGGLE_H_
#define TOGGLE_H_
#include "Arduino.h"

/* Clock generation
    Warning 1: Only use one of the A or B pins on a given timer.
               If both are attempted, then only the last one remains active.
    Warning 2: Arduino delay, millis, and micros depend on timer 0
    Warning 3: Arduino tone() uses timer
    
    The timers 0 and 2 will produce clocks within these bounds:
    Resolution of 2 / F_CPU (125ns on Uno)
    Minimum clock: Timer 0 & 2, clock of F_CPU / 512 (31250Hz on Uno)
                   Timer 1, clock of F_CPU / 131072 (about 123Hz on Uno)
    Maximum clock of F_CPU / 2
    
    Toggling on compare match is like having a automatic /2 prescaler
    The formula for output clock is: F_CPU / (OCR + 1) / 2
   
    Each macro sets:
       CTC mode (matching on relevant OCR)
       Appropriate OCR pin to toggle mode
       Prescaler to /1
       OCR value to the one supplied
       
    Timer pin map for the Arduino Uno
        |Arduino|
     AVR|digital|   AVR
    name|    pin|port|bit
    OC0A       6    D   6
    OC0B       5    D   5
    OC1A       9    B   1
    OC1B      10    B   2
    OC2A      11    B   3
    OC2B       3    D   3
*/

// Timer 0
inline void toggle_OC0A(double f) {
    TCCR0A = bit(COM0A0) | bit(WGM01);
    TCCR0B = bit(CS00);
    OCR0A = F_CPU / (f * 2) - 1;
}


inline void toggle_OC0B(double f) {
    TCCR0A = bit(COM0B0) | bit(WGM01);
    TCCR0B = bit(CS00);
    OCR0A = F_CPU / (f * 2) - 1;
}


// Timer 1
inline void toggle_OC1A(double f) {
    TCCR1A = bit(COM1A0);
    TCCR1B = bit(WGM12) | bit(CS10);
    OCR1A = F_CPU / (f * 2) - 1;
}


inline void toggle_OC1B(double f) {
    TCCR1A = bit(COM1B0);
    TCCR1B = bit(WGM12) | bit(CS10);
    OCR1A = F_CPU / (f * 2) - 1;
}


// Timer 2
inline void toggle_OC2A(double f) {
    TCCR2A = bit(COM2A0) | bit(WGM21);
    TCCR2B = bit(CS20);
    OCR2A = F_CPU / (f * 2) - 1;
}


inline void toggle_OC2B(double f) {
    TCCR2A = bit(COM2B0) | bit(WGM21);
    TCCR2B = bit(CS20);
    OCR2A = F_CPU / (f * 2) - 1;
}

// include guard
#endif
