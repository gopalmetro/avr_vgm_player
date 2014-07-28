/* Dependencies */
#include <avr/io.h>     // For I/O and other AVR registers
#include <util/delay.h> // For timing
#include <avr/pgmspace.h>
#include "tables.h"

/* Pinmap (Arduino UNO compatible) */
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



// MIDI input variables
byte channel;
byte pitch;
byte velocity;
byte ccnumber;
byte ccvalue;
byte ccvalue2;
byte bendLSB;
byte bendMSB;
byte rstat;
int dataIn;
int flag_previous = 0;
word pitchFrequency;
byte pitchMSB;
byte pitchLSB;

void blinkTest(int numBlinks = 1, int LEDHighTime = 50, int LEDLowTime = 50) {
  for (int i = 0; i < numBlinks; i++) {
    digitalWrite(13,HIGH);
    delay(LEDHighTime);
    digitalWrite(13,LOW);
    delay(LEDLowTime);
  }
}

static void write_ym(uint8_t data) {

	YM_CTRL_PORT &= ~_BV(YM_CS); // CS LOW
	//YM_DATA_PORT = data;
        YM_DATA_PORT = DDATA(data) | (YM_DATA_PORT & B00000011);
        PORTB = BDATA(data) | (PORTB & B11110011);
	_delay_us(1);
	YM_CTRL_PORT &= ~_BV(YM_WR); // Write data
	_delay_us(5);
	YM_CTRL_PORT |= _BV(YM_WR);
	_delay_us(5);
	YM_CTRL_PORT |= _BV(YM_CS); // CS HIGH
}

static void setreg(uint8_t reg, uint8_t data) {
	YM_CTRL_PORT &= ~_BV(YM_A0); // A0 low (select register)
	write_ym(reg);
	YM_CTRL_PORT |= _BV(YM_A0);  // A0 high (write register)
	write_ym(data);
}

void setup() {
  Serial.begin(57600);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  
/* Pins setup */
	YM_CTRL_DDR |= _BV(YM_IC) | _BV(YM_CS) | _BV(YM_WR) | _BV(YM_RD) | _BV(YM_A0) | _BV(YM_A1);
	//YM_DATA_DDR = 0xFF;
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
	setreg(0x22, 0x00); // LFO off
	setreg(0x27, 0x00); // Note off (channel 0) ???
	//setreg(0x28, 0x01); // Note off (channel 1) ???
	//setreg(0x28, 0x02); // Note off (channel 2) ???
	//setreg(0x28, 0x04); // Note off (channel 3) ???
	//setreg(0x28, 0x05); // Note off (channel 4) ???
	//setreg(0x28, 0x06); // Note off (channel 5) ???
	setreg(0x2B, 0x00); // DAC off
	setreg(0x30, 0x71); // Detune; Multiple
	setreg(0x34, 0x0D); // Detune; Multiple
	setreg(0x38, 0x33); // Detune; Multiple
	setreg(0x3C, 0x01); // DT1/MUL Detune; Multiple
	setreg(0x40, 0x23); // Total level Operator 1 Ch1
	setreg(0x44, 0x2D); // Total level Operator 2 Ch1
	setreg(0x48, 0x26); // Total level Operator 3 Ch1
	setreg(0x4C, 0x00); // Total level Operator 4 Ch1
	setreg(0x50, 0x5F); // RS/AR Rate Scaling; Attack Rate
	setreg(0x54, 0x99); // RS/AR Rate Scaling; Attack Rate
	setreg(0x58, 0x5F); // RS/AR Rate Scaling; Attack Rate
	setreg(0x5C, 0x94); // RS/AR Rate Scaling; Attack Rate
	setreg(0x60, 0x05); // AM/D1R First Decay Rate; Amplitude Modulation
	setreg(0x64, 0x05); // AM/D1R First Decay Rate; Amplitude Modulation
	setreg(0x68, 0x05); // AM/D1R First Decay Rate; Amplitude Modulation
	setreg(0x6C, 0x07); // AM/D1R First Decay Rate; Amplitude Modulation AM/D1R
	setreg(0x70, 0x02); // D2R Secondary Decay Rate
	setreg(0x74, 0x02); // D2R Secondary Decay Rate
	setreg(0x78, 0x02); // D2R Secondary Decay Rate
	setreg(0x7C, 0x02); // D2R Secondary Decay Rate
	setreg(0x80, 0x11); // D1L/RR Secondary Amplitude; Release Rate
	setreg(0x84, 0x11); // D1L/RR Secondary Amplitude; Release Rate
	setreg(0x88, 0x11); // D1L/RR Secondary Amplitude; Release Rate
	setreg(0x8C, 0xA6); // D1L/RR Secondary Amplitude; Release Rate
	setreg(0x90, 0x00); // Proprietary
	setreg(0x94, 0x00); // Proprietary
	setreg(0x98, 0x00); // Proprietary
	setreg(0x9C, 0x00); // Proprietary
	setreg(0xB0, 0x30); // Feedback/algorithm
	setreg(0xB4, 0xC0); // Both speakers on
	setreg(0x28, 0x00); // Key off
        //setreg(0xA4, 0x22); // Frequency MSB
        //setreg(0xA0, 0x69); // Frequency LSB
        
  blinkTest(3,200,200);
  //blinkTest(3,400,200);
  //blinkTest(3,200,200);
}

void loop() { 
    if(Serial.available() > 0) {

    dataIn = Serial.read();
    if(dataIn < 0x80 && flag_previous == 0) {
      doMidiIn(rstat);
    }
    doMidiIn(dataIn);
  }
}

void noteON(byte channel, byte pitch, byte velocity) {
  if(velocity > 0) {
      pitchFrequency = pgm_read_word(&PITCHTABLE[pitch]); //call the pitch table in tables.h to get the appropriate pitch word (word=2 bytes)
      pitchMSB = (pitchFrequency & 0x3F00) >> 8; //extract the most significant byte from the pitch word, shift the byte down and assign it to the pitchMSB byte variable
      pitchLSB = pitchFrequency & 0xFF; //mask out the most significant byte using 0xFF, then assign the least significant byte to pitchLSB
      setreg(0xA4, pitchMSB); //send the note frequency MSB to the Block/Freqency MSB register on the YM2612 the note value
      setreg(0xA0, pitchLSB); //send the note frequency LSB to the Frequency LSB register on the YM2612
      setreg(0x28, 0xF0); // Play a note ON; need to map this value to the correct algorithm "Slot(s) 
  }
  
  else {
      setreg(0x28, 0x00);
  }
}

void noteOff(byte channel, byte pitch, byte velocity) {
      pitchFrequency = pgm_read_word(&PITCHTABLE[pitch]);
      pitchMSB = (pitchFrequency & 0x3F00) >> 8; //extract the most significant byte from the pitch word, shift the byte down and assign it to the pitchMSB byte variable
      pitchLSB = pitchFrequency & 0xFF; //mask out the most significant byte using 0xFF, then assign the least significant byte to pitchLSB
      setreg(0xA4, pitchMSB); //send the note frequency MSB to the Block/Freqency MSB register on the YM2612 the note value
      setreg(0xA0, pitchLSB); //send the note frequency LSB to the Frequency LSB register on the YM2612
      setreg(0x28, 0x00); // Play a note ON; need to map the velocity value to the correct algorithm "Slot(s)

}


void CC(byte channel, byte ccnumber, byte ccvalue) {
  switch (channel) {  
    // YM2612 FM Channels
    case 0:
    case 3:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    case 1:
    case 4:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    case 2:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    // YM2612 DAC
    case 5:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    // SN764889
    case 6:
    case 7:
    case 8:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    // SN76489 Noise Channel
    case 9:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    // YM2612 CH3 Special Mode
    case 10:
    case 11:
    case 12:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
      break;
    // Everything Else
    default:
      switch (ccnumber) {
        case 00:
          break;
        case 11:
          break;
        default:
          break;
      }
  }
      
}

void pitchBend(byte channel, unsigned int bend_usb) {
}






void doMidiIn(byte data) {
    
    // running status set
   
   if((data >= 0x80) && (data < 0xf0) && (flag_previous == 0)) {
     rstat = data;
   }
   
    // deal with note on
    if((data >= 0x90) && (data < 0xa0) && (flag_previous == 0)) {
      channel = data & B00001111;
      // Put Part 1 and Part 2 Select code here (is this the best place? Also, doublecheck compatibility with YMControl        
      flag_previous = 1;
    }
    else if((data < 0x80) && (flag_previous == 1)) {     
      pitch = data;
      flag_previous = 2;
    }
    else if((data < 0x80) && (flag_previous == 2)) {
      velocity = data;
      noteON(channel, pitch, velocity);
      flag_previous = 0;
    }
    // done with note on

    // deal with note off (as discrete status byte)
    else if((data >= 0x80) && (data < 0x90) && (flag_previous == 0)) {
      channel = data & B00001111;
      flag_previous = -1;
    }
    else if((data < 0x80) && (flag_previous == -1)) {
      pitch = data;
      flag_previous = -2;
    }
    else if((data < 0x80) && (flag_previous == -2)) {
      velocity = data;
      noteOff(channel, pitch, velocity);
      flag_previous = 0;
    }
    // done with note off (as discrete status byte)

    // deal with cc data
    else if((data >= 0xb0) && (data < 0xc0) && (flag_previous == 0)) {
      channel = data & B00001111;
      flag_previous = 3;
    }
    else if((data < 0x80) && (flag_previous == 3)) {
      ccnumber = data;
      flag_previous = 4;
    }
    else if((data < 0x80) && (flag_previous == 4)) {
      ccvalue = data;
        CC(channel, ccnumber, ccvalue);
      flag_previous = 0;
    }
    // done with cc data

    // deal with bend data
    else if((data >= 0xe0) && (data < 0xf0) && (flag_previous == 0)) {
      channel = data & B00001111;
      flag_previous = 5;
    }
    else if((data < 0x80) && (flag_previous == 5)) {
      bendLSB = data;
      flag_previous = 6;
    }
    else if((data < 0x80) && (flag_previous == 6)) {
      bendMSB = data;
      pitchBend(channel, bendLSB + (bendMSB << 7));
      flag_previous = 0;
    }
    // done with bend data
   
} 
