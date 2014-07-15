/**
 * YM2612 test code for AVR.
 * 
 * This program is a simple test code for the YM2612 FM sound chip using an AVR ATmega328p mcu.
 * This program configure the YM2612 to sound like a "grand piano" and play note on / note off in loop.
 * For more informations about wiring please see: http://en.wikipedia.org/wiki/Yamaha_YM2612
 * For more informations about YM2612 registers please see: http://www.smspower.org/maxim/Documents/YM2612
 *
 * @remarks This test code is heavly based on Furrtek's YM2612 test code. Big thanks Furrtek for the help !
 * @warning This test code is made to run on an ATmega328/ATmega168 mcu with a 16MHz external crystal.
 * 
 * @author Fabien Batteix <skywodd@gmail.com>
 * @link http://skyduino.wordpress.com My Blog about electronics
 
 
 * JULY 2014 - Gopal Metro added the YM2612 instructions from http://www.smspower.org/maxim/Documents/YM2612
 * to the relevant areas of the code. ALL Registers were also added and commented out.  To activate a Register
 * simply uncomment and input the desired values. 
 */

/* Dependencies */
#include <avr/io.h>     // For I/O and other AVR registers
#include <util/delay.h> // For timing
#include "MIDI_parser.h" //MIDI

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

/* ----- BIG WARNING FOR ARDUINO USERS -----
 * Normally ICSP port is used to program the AVR chip (see the makefile for details). 
 * So, if you use an arduino UNO board to run this test code BE VERY CAREFULL.
 * If you don't known what you make you will be in trouble.
 *
 * To avoid problems you can compile and upload this code using the Arduino IDE BUT you will need to disconnect pins Rx/Tx before upload !
 */

/**
 * Send raw data to the YM2612
 * 
 * @author Furrtek
 * @param data Data to write
 */
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

/**
 * Write data into a specific register of the YM2612
 *
 * @author Furrtek
 * @param reg Destination register address
 * @param data Data to write
 */
static void setreg(uint8_t reg, uint8_t data) {
	YM_CTRL_PORT &= ~_BV(YM_A0); // A0 low (select register)
	write_ym(reg);
	YM_CTRL_PORT |= _BV(YM_A0);  // A0 high (write register)
	write_ym(data);
}

/** Program entry point */
int main(void) {
  /* activate MIDI Serial input - Gopal */
  Serial.begin(38400);
  midiParser parser;  //-Make a MIDI parser
  /* END activate MIDI*/

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
	setreg(0x60, 0x05); // AM/D1R Firsty Decay Rate; Amplitude Modulation
	setreg(0x64, 0x05); // AM/D1R Firsty Decay Rate; Amplitude Modulation
	setreg(0x68, 0x05); // AM/D1R Firsty Decay Rate; Amplitude Modulation
	setreg(0x6C, 0x07); // AM/D1R Firsty Decay Rate; Amplitude Modulation AM/D1R
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
//	setreg(0xB0, 0x32); // Feedback/algorithm
        setreg(0xB0, 0x32);
	setreg(0xB4, 0xC0); // Both speakers on
	setreg(0x28, 0x00); // Key off

//      START FREQUENCY SETTINGS FOR ALL CHANNELS

/*      ----Registers A0H-AFH - FREQUENCY INSTRUCTIONS----

        
                |   D7   |   D6   |   D5   |   D4   |	D3   |	D2   |	D1   |	D0   |
        A0H+	|Frequency number LSB -------------------------------------          |
        A4H+	|        |        | Block                    |Frequency number MSB   |
        A8H+	|Ch3 supplementary frequency number                                  |
        ACH+		          |Ch3 supplementary block   |Ch3 suppl freq number  |
        
        Channel 1′s frequency is in A0 and A4H.
        Channel 2′s frequency is in A1 and A5H.
        Channel 3′s frequency, if it is in normal mode (please see page 12), is in A2 and A6H.
        If Channel 3 is in special mode:
        Operator 1′s frequency is in A2 and A6H
        Operator 2′s frequency is in A8 and ACH
        Operator 3′s frequency is in A9 and ADH
        Operator 4′s frequency is in AA and AEH
        
        The frequency is a 14-bit number that should be set high byte, low byte (e.g. A4H then A0H). 
        The highest 3 bits, called the “block”, give the octave. 
        The next 10 bits give the position in the octave, and a possible 12-tone sequence is:
        Low                                                              High
        617   653   692   733   777   823   872   924   979  1037  1099  1164
           635   372   392   755   800   847   898   951   1008  1131  1199
           (36)  (39)  (41)  (44)  (46)  (49)  (52)  (55)  (58)  (62)  (70)
        (all numbers in base 10)
        This sequence should be used inside each octave.
*/

//      CHANNEL 1 FREQUENCY
	setreg(0xA4, 0x22); // Set CH1 frequency MSB first
	setreg(0xA0, 0x69); // Set CH1 frequency LSB second

//      CHANNEL 2 FREQUENCY
//	setreg(0xA5, 0x22); // Set CH2 frequency MSB first
//	setreg(0xA1, 0x69); // Set CH2 frequency LSB second

//      CHANNEL 3 FREQUENCY NORMAL MODE
//	setreg(0xA6, 0x22); // Set CH3 Normal Mode frequency MSB first
//	setreg(0xA2, 0x69); // Set CH3 Normal Mode frequency LSB second

//      CHANNEL 3 SPECIAL MODE
/*	setreg(0xA6, 0x22); // Set CH3 Operator 1 Frequency MSB first
	setreg(0xA2, 0x69); // Set CH3 Operator 1 Frequency LSB second
	setreg(0xAC, 0x22); // Set CH3 Operator 2 Frequency MSB first
	setreg(0xA8, 0x69); // Set CH3 Operator 2 Frequency LSB second
	setreg(0xAD, 0x22); // Set CH3 Operator 3 Frequency MSB first
	setreg(0xA9, 0x69); // Set CH3 Operator 3 Frequency LSB second
	setreg(0xAE, 0x22); // Set CH3 Operator 4 Frequency MSB first
	setreg(0xAA, 0x69); // Set CH3 Operator 4 Frequency LSB second
*/

/* Program loop */
	for(;;) {
//		setreg(0xA4, random(B00000000,B00111111)); // Set frequency Octave
//              setreg(0xA0, random(B00000000,B11111111)); // Set Frequency Pitch
//	        setreg(0xA4, 0x22); // Set CH1 frequency MSB first
//	        setreg(0xA0, 0x69); // Set CH1 frequency LSB second
//		_delay_ms(100);
//		setreg(0x28, 0xF0); // Key on
//		_delay_ms(3000);
//		setreg(0x28, 0x00); // Key off

          unsigned char voice;
          while(Serial.available())
          {
            if(parser.update(Serial.read()))  //-Feed MIDI stream to parser and execute commands
            {
              switch(parser.midi_cmd)
              {
                //*********************************************
                // Handle MIDI notes
                //*********************************************
              case 0x90: //-Channel 1 (voice 0)
              case 0x91: //-Channel 2 (voice 1)
              case 0x92: //-Channel 3 (voice 2)
              case 0x93: //-Channel 4 (voice 3)
        
                voice = parser.midi_cmd-0x90;
                if(parser.midi_2nd)  //-Velocity not zero (could implement NOTE_OFF here)
		{
                        setreg(0xA4, random(B00000000,B00111111)); // Set frequency Octave
                        setreg(0xA0, random(B00000000,B11111111)); // Set Frequency Pitch
        	        setreg(0xA4, 0x22); // Set CH1 frequency MSB first
        	        setreg(0xA0, 0x69); // Set CH1 frequency LSB second
        		setreg(0x28, 0xF0); // Key on
        		_delay_ms(3000);
        		setreg(0x28, 0x00); // Key off
                }
                break;
        
                //*********************************************
                // Handle MIDI controllers
                //*********************************************
              /*case 0xb0:  //-Channel 1 (voice 0)
              case 0xb1:  //-Channel 2 (voice 1)
              case 0xb2:  //-Channel 3 (voice 2)
              case 0xb3:  //-Channel 4 (voice 3)
                voice=parser.midi_cmd-0xb0;
                switch(parser.midi_1st)  //-Controller number
                {
                case 13:  //-Controller 13 
                  edgar.setWave(voice,parser.midi_2nd/21);
                  break;
                case 12:  //-Controller 12
                  edgar.setEnvelope(voice,parser.midi_2nd/32);
                  break;   
                case 10:  //-Controller 10
                  edgar.setLength(voice,parser.midi_2nd);
                  break;  
                case 7:   //-Controller 7
                  edgar.setMod(voice,parser.midi_2nd);
                  break;
                }
                break;*/
              }
            }
          }
	}

/* Compiler fix */
	return 0;
}

