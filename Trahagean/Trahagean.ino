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

byte statusByte;
byte statusCommand;
byte MIDIChannel;
byte databyte1; //Register and offset
byte databyte2; //Actual value
byte algorithm;
uint16_t pitch;
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

void setup(){
  /* activate MIDI Serial input - Gopal */
  Serial.begin(38400);
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
	setreg(0x27, 0x00); // Note off (channel 0)
	setreg(0x28, 0x01); // Note off (channel 1)
	setreg(0x28, 0x02); // Note off (channel 2)
	setreg(0x28, 0x04); // Note off (channel 3)
	setreg(0x28, 0x05); // Note off (channel 4)
	setreg(0x28, 0x06); // Note off (channel 5)
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

/*
// velocityMap() will map the incoming databyte2 velocity 
// to the appropriate Operator "Slots" based on whichever 
// Operator Alogrithm is chosen in Register 0xB0,0xB1 and 0xB2
void velocityMap() {
  switch (algorithm)
    default:
    break;
}
*/



void MIDIinput() {
  do {    
    if (Serial.available()) {
      statusByte = Serial.read(); // get first byte of MIDI data from computer
      statusCommand = ((statusByte & B11110000) >> 4);
      MIDIChannel = statusByte & B00001111;

      switch (statusCommand) {
        case 0x9: //NoteOn
          /*if (!((databyte1 = Serial.read()) & B10000000) && !((databyte2 = Serial.read()) & B10000000)) {  
          }*/        
          //blinkTest(1,10,1);
          databyte1 = Serial.read();  //note value
          databyte2 = Serial.read();   //note velocity
          pitch = pgm_read_word(&PITCHTABLE[databyte1]); //call the pitch table in tables.h to get the appropriate pitch word (word=2 bytes)
          pitchMSB = (pitch & 0x3F00) >> 8; //extract the most significant byte from the pitch word, shift the byte down and assign it to the pitchMSB byte variable
          pitchLSB = pitch & 0xFF; //mask out the most significant byte using 0xFF, then assign the least significant byte to pitchLSB
          setreg(0xA4, pitchMSB); //send the note frequency MSB to the Block/Freqency MSB register on the YM2612 the note value
          setreg(0xA0, pitchLSB); //send the note frequency LSB to the Frequency LSB register on the YM2612
          setreg(0x28, 0xF0); // Play a note ON; need to map this value to the correct algorithm "Slot(s)"
          
          break;

        case 0x8: //NoteOff
         //blinkTest(3,100,10);
         setreg(0x28, 0x00); // Stop the note
         databyte1 = Serial.read();  //note value
         databyte2 = Serial.read();   //note velocity
          break;

        case 0xB: //Continuous Controller (CC)
         // blinkTest(1, 100, 100);
         databyte1 = Serial.read(); // CC number
         databyte2 = Serial.read(); // CC value
/*
           switch (databyte1)
             case 0x14: // testing CC information
             blinkTest(2, 10, 50);
             setreg(0xB0, (databyte2 & B00000111)); //Changing the algorithm
             break;
*/
         break;
        default:
        break;          
        }
      }
  }
  while (Serial.available() >= 2);//when at least three bytes available
}

void loop() {
  MIDIinput();
}
