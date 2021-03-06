/************************************************************************/
/* toggle.h - generate clock signals                                    */
/************************************************************************/

#ifndef TOGGLE_H_
#define TOGGLE_H_

#include "Arduino.h"

/* Clock generation - targeting ATMega328p or similar AVR
    Warning 1: Only use one of the A or B pins on a given timer.
               If both are attempted, then only the latter one remains active.
    Warning 2: Arduino delay, millis, and micros depend on timer 0
    Warning 3: Arduino servo library depends on timer 1
    Warning 4: Arduino tone depends on timer 2
    
    The timers will produce clocks within the following traits:
    Resolution of 2 / F_CPU (125ns on Uno)
    Timer 0 & 2, minimum clock of F_CPU / 512 (31250Hz on Uno)
    Timer 1, minimum clock of F_CPU / 131072 (about 123Hz on Uno)
    Maximum clock of F_CPU / 2
    
    Each function sets:
        CTC mode
        One OC pin to toggle mode
        Prescaler to /1
        OCR value to the indicated frequency, 'f',
            rounded up to the nearest 125ns increment.
       
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
    
    Example for Uno putting 4Mhz on pin 5:
    void setup() {
        pinMode(5, OUTPUT);
        toggle_OC0B(4000000.0);
    }
    
    
    Beware: specified frequencies are rounded up to the nearest value
    the base clock supports, such that, on an Uno, 2700000 runs at 4000000.
    Practical clocks on an Uno (at F_CPU = 16000000 or 125ns resolution):
    8000000.000000,
    4000000.000000,
    2666666.666666,
    2000000.000000,
    1600000.000000,
    1333333.333333,
    1142857.142857,
    1000000.000000,
     888888.888888,
     800000.000000,
     727272.727272,
     666666.666666,
     615384.615384,
     571428.571428,
     533333.333333,
     500000.000000,
    etc. If you need more accuracy, use an external clock!
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
