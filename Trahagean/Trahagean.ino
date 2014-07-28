/* Dependencies */
#include <avr/io.h>     // For I/O and other AVR registers
#include <util/delay.h> // For timing
#include <avr/pgmspace.h>
#include "tables.h"

/* Pin map (Arduino UNO compatible) */
#define YM_IC (5) // PC5 (= pin A5 for Arduino UNO)
#define YM_CS (4) // PC4 (= pin A4 for Arduino UNO)
#define YM_WR (3) // PC3 (= pin A3 for Arduino UNO)
#define YM_RD (2) // PC2 (= pin A2 for Arduino UNO)
#define YM_A0 (1) // PC1 (= pin A1 for Arduino UNO)
#define YM_A1 (0) // PC0 (= pin A0 for Arduino UNO)
#define YM_CTRL_DDR DDRC
#define YM_CTRL_PORT PORTC
#define YM_DATA_DDR DDRD
#define YM_DATA_PORT PORTD // Whole PORT D for data bus (= pins D0 to D7 for Arduino UNO)
#define YM_MCLOCK (1) // PB1 = OC1A (= pin D9 for Arduino UNO)
#define YM_MCLOCK_DDR DDRB

//PD0 & PD1 for serial i/o, using PD2-PD7 as YM 0-5
#define DDATA(d) ((d << 2) & B11111100)
// PB2 PB3 as YM 6-7
#define BDATA(d) ((d >> 4) & B00001100)

#define YM2612_OP1 B0001
#define YM2612_OP2 B0010
#define YM2612_OP3 B0100
#define YM2612_OP4 B1000


void blinkTest(int numBlinks = 1, int LEDHighTime = 50, int LEDLowTime = 50) {
	for (int i = 0; i < numBlinks; i++) {
		digitalWrite(13,HIGH);
		delay(LEDHighTime);
		digitalWrite(13,LOW);
		delay(LEDLowTime);
	}
}

class YM2612 {
    byte slotOperators[6]; // bit-mask to identify operators in algorithm "slots", per channel

    void write(uint8_t data) {
	    YM_CTRL_PORT &= ~_BV(YM_CS); // CS LOW
	    YM_DATA_PORT = DDATA(data) | (YM_DATA_PORT & B00000011);
	    PORTB = BDATA(data) | (PORTB & B11110011);
	    _delay_us(1);
	    YM_CTRL_PORT &= ~_BV(YM_WR); // Write data
	    _delay_us(5);
	    YM_CTRL_PORT |= _BV(YM_WR);
	    _delay_us(5);
	    YM_CTRL_PORT |= _BV(YM_CS); // CS HIGH
    }


    void setreg(uint8_t reg, uint8_t data, byte part) {
	    if (part) {
	        YM_CTRL_PORT |= _BV(YM_A1); // A1 high (part II, YM channels 456)
            YM_CTRL_PORT &= ~_BV(YM_A0); // A0 low (select register)
        }
	    else
	        YM_CTRL_PORT &= ~(_BV(YM_A1) | _BV(YM_A0)); // A0 low (select register), A1 low (part I, global and YM channels 123)
	    write(reg);
	    YM_CTRL_PORT |= _BV(YM_A0);  // A0 high (write register)
	    write(data);
    }


    public:
    YM2612(){
        /* no-op */
    }


    void begin() {
        /* Pins setup */
        YM_CTRL_DDR |= _BV(YM_IC) | _BV(YM_CS) | _BV(YM_WR) | _BV(YM_RD) | _BV(YM_A0) | _BV(YM_A1);
        YM_DATA_DDR = B11111100;
        YM_MCLOCK_DDR |= _BV(YM_MCLOCK) | B00001100;
        YM_CTRL_PORT |= _BV(YM_IC) | _BV(YM_CS) | _BV(YM_WR) | _BV(YM_RD); /* IC, CS, WR and RD HIGH by default */
        YM_CTRL_PORT &= ~(_BV(YM_A0) | _BV(YM_A1)); /* A0 and A1 LOW by default */

        /* F_CPU / 2 clock generation */
        TCCR1A = _BV(COM1A0);            /* Toggle OCA1 on compare match */
        TCCR1B = _BV(WGM12) | _BV(CS10); /* CTC mode with prescaler /1 */
        TCCR1C = 0;                      /* Flag reset */
        TCNT1 = 0;                       /* Counter reset */
        OCR1A = 0;                       /* Divide base clock by two */

        /* Reset YM2612 */
        YM_CTRL_PORT &= ~_BV(YM_IC);
        _delay_ms(10);
        YM_CTRL_PORT |= _BV(YM_IC);
        _delay_ms(10);

        /* YM2612 Test code */
        setreg(0x22, 0x00, 0); // LFO off
        /* make sure notes are off */
        operatorKey(0, 0);
        operatorKey(1, 0);
        operatorKey(2, 0);
        operatorKey(3, 0);
        operatorKey(4, 0);
        operatorKey(5, 0);
        setreg(0x27, 0x00, 0); // timers, special mode off
        setreg(0x2B, 0x00, 0); // DAC off
        /* init voices */
        voice(0);
        voice(1);
        voice(2);
        voice(3);
        voice(4);
        voice(5);
        setreg(0x90, 0x00, 0); // Proprietary
        setreg(0x94, 0x00, 0); // Proprietary
        setreg(0x98, 0x00, 0); // Proprietary
        setreg(0x9C, 0x00, 0); // Proprietary
    }

    
    void operatorKey(byte channel, byte bitfield) {
        if (bitfield)
            digitalWrite(13, HIGH); //debug
        else
            digitalWrite(13, LOW); //debug
	    setreg(0x28, ((bitfield & B1111) << 4) | (channel < 3 ? channel : channel % 3 + 4), 0); //skip over 011
    }


    void frequency(byte channel, uint16_t pitch) {
	    byte lsbReg =  channel <= 5 ? 0xA0 : 0xAC; //if higher than 5 then use the special mode registers
	    byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	    byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
	    setreg(lsbReg + chanOffset + 4, pitch >> 8, part); //pitch MSB first, MSB register is LSB+4
	    setreg(lsbReg + chanOffset, pitch & 0xFF, part); //pitch LSB
    }


    void level(byte channel, byte val)
    {
        if (channel < 6) {
	        byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	        byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
            setreg(0x40 + chanOffset, val, part); // Operator 1
/* TODO FIXME
            if (slotOperators[channel] & YM2612_OP1)
	            setreg(0x40 + chanOffset, val, part); // Operator 1
            if (slotOperators[channel] & YM2612_OP2)
	            setreg(0x44 + chanOffset, val, part); // Operator 2
            if (slotOperators[channel] & YM2612_OP3)
	            setreg(0x48 + chanOffset, val, part); // Operator 3
            if (slotOperators[channel] & YM2612_OP4)
	            setreg(0x4C + chanOffset, val, part); // Operator 4
*/
        }
    }


    void voice(byte channel)
    {
	    byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	    byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
	    setreg(0x28 + chanOffset, 0x00, part); // Key off
	    setreg(0x30 + chanOffset, 0x71, part); // Detune; Multiple
	    setreg(0x34 + chanOffset, 0x0D, part); // Detune; Multiple
	    setreg(0x38 + chanOffset, 0x33, part); // Detune; Multiple
	    setreg(0x3C + chanOffset, 0x01, part); // DT1/MUL Detune; Multiple
	    setreg(0x40 + chanOffset, 0x23, part); // Total level Operator 1
	    setreg(0x44 + chanOffset, 0x2D, part); // Total level Operator 2
	    setreg(0x48 + chanOffset, 0x26, part); // Total level Operator 3
	    setreg(0x4C + chanOffset, 0x00, part); // Total level Operator 4
	    setreg(0x50 + chanOffset, 0x5F, part); // RS/AR Rate Scaling; Attack Rate
	    setreg(0x54 + chanOffset, 0x99, part); // RS/AR Rate Scaling; Attack Rate
	    setreg(0x58 + chanOffset, 0x5F, part); // RS/AR Rate Scaling; Attack Rate
	    setreg(0x5C + chanOffset, 0x94, part); // RS/AR Rate Scaling; Attack Rate
	    setreg(0x60 + chanOffset, 0x05, part); // AM/D1R First Decay Rate; Amplitude Modulation
	    setreg(0x64 + chanOffset, 0x05, part); // AM/D1R First Decay Rate; Amplitude Modulation
	    setreg(0x68 + chanOffset, 0x05, part); // AM/D1R First Decay Rate; Amplitude Modulation
	    setreg(0x6C + chanOffset, 0x07, part); // AM/D1R First Decay Rate; Amplitude Modulation AM/D1R
	    setreg(0x70 + chanOffset, 0x02, part); // D2R Secondary Decay Rate
	    setreg(0x74 + chanOffset, 0x02, part); // D2R Secondary Decay Rate
	    setreg(0x78 + chanOffset, 0x02, part); // D2R Secondary Decay Rate
	    setreg(0x7C + chanOffset, 0x02, part); // D2R Secondary Decay Rate
	    setreg(0x80 + chanOffset, 0x11, part); // D1L/RR Secondary Amplitude; Release Rate
	    setreg(0x84 + chanOffset, 0x11, part); // D1L/RR Secondary Amplitude; Release Rate
	    setreg(0x88 + chanOffset, 0x11, part); // D1L/RR Secondary Amplitude; Release Rate
	    setreg(0x8C + chanOffset, 0xA6, part); // D1L/RR Secondary Amplitude; Release Rate
        feedbackAlgorithm(channel, 6, 0);
	    setreg(0xB4 + chanOffset, 0xC0, part); // Both speakers on
    }


//keep track of which operators are in algorithm slots
    void feedbackAlgorithm(byte channel, byte fb, byte alg) {
        if (channel < 6) {
	        byte chanOffset = channel % 3; //channels are in sequence, and 012 overlap 345
	        byte part = (3 <= channel && channel <= 5); // part is 0 unless channel is 3 4 5
            alg &= B00000111;
            setreg(0xB0 + chanOffset, ((fb & B00000111) << 3) | alg, part); //feedback is 3 bits offset 3, algorithm is 3 bits no offset
/* algorithm : operators
    0-3 : 4
    4   : 2,4
    5,6 : 2,3,4
    7   : 1,2,3,4
*/
            slotOperators[channel] = YM2612_OP4
                | (alg >= 5 ? YM2612_OP3 : 0)
                | (alg >= 4 ? YM2612_OP2 : 0)
                | (alg == 7 ? YM2612_OP1 : 0);
        }
    }
};


class Megasynth {
    YM2612 ym;

    public:
    Megasynth() {
        ym = YM2612();
    }


    void begin() {
        pinMode(13,OUTPUT);
        digitalWrite(13,LOW);
        ym.begin();
    }


    void noteOn(byte channel, byte note, byte velocity) {
	    if (channel <= 5 || (10 <= channel && channel <= 12 )) {
		    ym.frequency(channel, pgm_read_word(&PITCHTABLE[note])); //use pitch table in tables.h to get the appropriate pitch word
		    if (channel <= 5)
                ym.level(channel, velocity);
    		    ym.operatorKey(channel, YM2612_OP1 | YM2612_OP2 | YM2612_OP3 | YM2612_OP4); //enable ALL the operators
	    }
    }


    void noteOff(byte channel, byte note, byte velocity) {
	    if (channel <= 5)
		    ym.operatorKey(channel, 0); //disable ALL the operators
    }
};


Megasynth synth = Megasynth();


void setup() {
    /* activate MIDI Serial input - Gopal */
    Serial.begin(38400);
    synth.begin();
    delay(200);
    blinkTest(3,200,200);
    //blinkTest(3,400,200);
    //blinkTest(3,200,200);
}

/*
// velocityMap() will map the incoming databyte2 velocity 
// to the appropriate Operator "Slots" based on whichever 
// Operator Algorithm is chosen in Register 0xB0,0xB1 and 0xB2
void velocityMap() {
  switch (algorithm)
    default:
    break;
}
*/


void loop() {
    while (Serial.available() >= 2) {
        byte statusByte;
        byte statusCommand;
        byte MIDIChannel;
        byte databyte1; //Register and offset
        byte databyte2; //Actual value
        statusByte = Serial.read();
        statusCommand = (statusByte & B11110000) >> 4;
        MIDIChannel = statusByte & B00001111;

        switch (statusCommand) {
            case 0x9:
            databyte1 = Serial.read(); //note value
            databyte2 = Serial.read(); //note velocity
            synth.noteOn(MIDIChannel, databyte1, databyte2);
            break;

            case 0x8:
            databyte1 = Serial.read(); //note value
            databyte2 = Serial.read(); //note velocity
            synth.noteOff(MIDIChannel, databyte1, databyte2);
            break;

            case 0xB: //Continuous Controller (CC)
            databyte1 = Serial.read(); // CC number
            databyte2 = Serial.read(); // CC value
/*
            switch (databyte1) {
                case 0x14: // testing CC information
                blinkTest(2, 10, 50);
                setreg(0xB0, (databyte2 & B00000111), 0); //Changing the algorithm
                break;
            }               
*/
            break;

            default:
            break;          
        }
    }
}

