/* Dependencies */
#include <avr/sleep.h>
#include "Arduino.h"
#include "toggle.h"

// flag for MegaSynth to dump freqs for verification purposes
//#define DUMP_FREQS
// initial tuning (default is 440)
//#define EQUAL_TEMPERAMENT_A4 440.0
#include "MegaSynth.h"

//#define USE_QD_PACKETIZER
//#define BAUDRATE MIDI_NATIVE_BAUDRATE
#define BAUDRATE MIDI_SOFTWARE_BAUDRATE


//rate for MIDI bridge software (for historical reasons this number is a multiple of 300)
#define MIDI_SOFTWARE_BAUDRATE 38400
//standard MIDI rate (1MHz/32, easier for embedded systems)
#define MIDI_NATIVE_BAUDRATE 31250


/************************************************************************/
/* BEGIN HACKY SN STUFF                                                    */
/************************************************************************/
#include <util/delay.h> // For timing

/**
 * SN76489 test code for AVR.
 * 
 * This program is a simple test code for the SN76489 PSG sound chip using an AVR ATmega328p mcu.
 * For more informations about wiring please see: http://members.casema.nl/hhaydn/howel/parts/76489.htm
 * For more informations about SN76489 registers please see: http://www.smspower.org/Development/SN76489
 *
 * @warning This test code is made to run on an ATmega328/ATmega168 mcu with a 16MHz external crystal.
 * 
 * @author Fabien Batteix <skywodd@gmail.com>
 * @link http://skyduino.wordpress.com My Blog about electronics
 */


/* Pin map (Arduino UNO compatible) */

#define SN76489_CE_PORT PORTC
#define SN76489_CE_DDR  DDRC
#define SN76489_CE_BIT  3
#define SN76489_WE_PORT PORTC
#define SN76489_WE_DDR  DDRC
#define SN76489_WE_BIT  2

//Pin map for data pins is a bit complicated:
//SN76489_D0 through SN76489_D3 map to PORTD pins 4-7 -- Uno digital pins 4-7
//SN76489_D4 through SN76489_D7 map to PORTB pins 2-5 -- Uno digital pins 10-13

#define SN76489_DATA_PORTD_MASK B11110000
#define SN76489_DATA_PORTD_BITBANG(b) ((b) << 4)

#define SN76489_DATA_PORTB_MASK B00111100
#define SN76489_DATA_PORTB_BITBANG(b) ((b) >> 2)


#if !defined(SN76489_DATA_PORTB_MASK)
    && !defined(SN76489_DATA_PORTC_MASK)
    && !defined(SN76489_DATA_PORTD_MASK)
#error "define at least one SN76489_DATA_PORT[BCD]_MASK"
#endif
#if defined(SN76489_DATA_PORTB_MASK) && !defined(SN76489_DATA_PORTB_BITBANG)
#error "SN76489_DATA_PORTB_MASK defined without SN76489_DATA_PORTB_BITBANG(b)"
#endif
#if defined(SN76489_DATA_PORTC_MASK) && !defined(SN76489_DATA_PORTC_BITBANG)
#error "SN76489_DATA_PORTC_MASK defined without SN76489_DATA_PORTC_BITBANG(b)"
#endif
#if defined(SN76489_DATA_PORTD_MASK) && !defined(SN76489_DATA_PORTD_BITBANG)
#error "SN76489_DATA_PORTD_MASK defined without SN76489_DATA_PORTD_BITBANG(b)"
#endif



/* ----- BIG WARNING FOR ARDUINO USERS -----
 * Normally ICSP port is used to program the AVR chip (see the makefile for details). 
 * So, if you use an arduino UNO board to run this test code BE VERY CAREFULL.
 * If you don't known what you make you will be in trouble.
 *
 * To avoid problems you can compile and upload this code using the Arduino IDE BUT you will need to disconnect pins Rx/Tx before upload !
 */

/** Macro for frequency register */
#define FREG_REG_VAL(f) (4000000UL / 32 / (f))

/* Register values */
#define ATTENUATION_2DB bit(0)
#define ATTENUATION_4DB bit(1)
#define ATTENUATION_8DB bit(2)
#define ATTENUATION_16DB bit(3)
#define ATTENUATION_OFF (15)

/* Noise controls */
#define NOISE_PERIODIC_FEEDBACK (0)
#define NOISE_WHITE_FEEDBACK bit(2)
#define NOISE_SHIFT_512 (0)
#define NOISE_SHIFT_1024 (1)
#define NOISE_SHIFT_2048 (2)
#define NOISE_SHIFT_TONE3 (3)

/**
 * Send raw data to the SN76489
 * 
 * @param data Data to write
 */
static void write_psg(uint8_t data) {
	//while(PSG_CTRL_PIN & _BV(PSG_READY)); // Wait for chip to be ready
//    Serial.print("write data: ");
//    Serial.println(data, BIN);
#ifdef SN76489_DATA_PORTB_MASK
//    Serial.print("B: ");
//    Serial.print(PORTB, BIN);
    PORTB = (PORTB & ~(SN76489_DATA_PORTB_MASK))
        | (SN76489_DATA_PORTB_BITBANG(data) & SN76489_DATA_PORTB_MASK);
//    Serial.print(" -> ");
//    Serial.println(PORTB, BIN);
#endif
#ifdef SN76489_DATA_PORTC_MASK
//    Serial.print("C: ");
//    Serial.print(PORTC, BIN);
    PORTC = (PORTC & ~(SN76489_DATA_PORTC_MASK))
        | (SN76489_DATA_PORTC_BITBANG(data) & SN76489_DATA_PORTC_MASK);
//    Serial.print(" -> ");
//    Serial.println(PORTC, BIN);
#endif
#ifdef SN76489_DATA_PORTD_MASK
//    Serial.print("D: ");
//    Serial.print(PORTD, BIN);
    PORTD = (PORTD & ~(SN76489_DATA_PORTD_MASK))
        | (SN76489_DATA_PORTD_BITBANG(data) & SN76489_DATA_PORTD_MASK);
//    Serial.print(" -> ");
//    Serial.println(PORTD, BIN);
#endif
//    Serial.flush();
	SN76489_WE_PORT &= ~bit(SN76489_WE_BIT); // WE LOW (latch)
	SN76489_CE_PORT &= ~bit(SN76489_CE_BIT); // CS LOW
	_delay_us(150);
	SN76489_CE_PORT |= bit(SN76489_CE_BIT); // CS HIGH
	SN76489_WE_PORT |= bit(SN76489_WE_BIT); // WE HIGH
}

/**
 * Set the value of a channel (with latch)
 *
 * @param reg Register address
 * @param data Data to write
 */
static void setreg(uint8_t reg, uint8_t data) {
    noInterrupts();
	write_psg(0x80 | ((reg & 0x07) << 4) | (data & 0x0F));
    interrupts();
}


/**
 * Set the frequency of a channel
 *
 * @param channel Channel number (0, 1, 2)
 * @param frequency Frequency in Hz
 */
static void setFrequency(uint8_t channel, uint16_t frequency) {
	uint16_t f = FREG_REG_VAL(frequency);
	setreg(2 * channel, f & 0x0F);
	write_psg((f >> 4) & 0x3F);
}




/************************************************************************/
/* END HACKY SN STUFF                                                   */
/************************************************************************/



MegaSynth synth;


#ifdef USE_QD_PACKETIZER
// Quick and dirty packetizer
// It doesn't use any MACROS and can be plopped in anywhere
// Technical limitations:
// * Packets smaller than size 3 will get hung until the next command comes in
// * Only the first two data bytes of a 0xF0 command will get through
// * No sanity checks for wrong sized packets (reason behind first limitation)
void qdMidiPacketizer(void (*midiPacketParser)(const byte *), int inByte){
    static byte packet[3];
    static byte count = 0; // no bytes read yet
    if (inByte < 0) {  // is inByte an error?
        return; //abort!
    }
    if (bitRead(inByte, 7) == 1) { // MSB set = status byte, new packet start
        if (count > 0) { // is there an existing packet?
            midiPacketParser(packet); //PARSE THAT PACKET
        }
        count = 1; // inByte is new packet's first byte
        packet[0] = inByte; // new status byte
    }
    else if (count >= 1) { // has a packet been started?
        // append data byte to packet
        // adding inByte immediately increases count by 1
        // but first use stale value since it's already array index: count - 1
        packet[count] = inByte;
        ++count; //increment after using the stale value
        if (count == 3) { // process a full packet immediately
            midiPacketParser(packet); //PARSE THAT PACKET
            count = 0; // reset packet
        }
    }
}
#else
MidiPacketizer packetizer;
#endif

void blinkTest(byte numBlinks = 1, word LEDHighTime = 50, word LEDLowTime = 50) {
	for (int i = 0; i < numBlinks; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(LEDHighTime);
		digitalWrite(LED_BUILTIN, LOW);
		delay(LEDLowTime);
	}
}


void setup() {
    /* activate MIDI Serial input - Gopal */
    Serial.begin(BAUDRATE);
        
    //SN Clock on Uno's D9
    toggle_OC1A(4000000.0); // 4MHz
    pinMode(9, OUTPUT);

    //YM Clock on Uno's D3
    toggle_OC2B(8000000.0); // 8MHz
    pinMode(3, OUTPUT);
    
    synth.begin();
    delay(200);
    blinkTest(3,200,200);
    //blinkTest(3,400,200);
    //blinkTest(3,200,200);
    set_sleep_mode(SLEEP_MODE_IDLE);


	/* SN Pins setup */
	SN76489_WE_DDR |= bit(SN76489_WE_BIT);
	SN76489_CE_DDR |= bit(SN76489_CE_BIT);

	#ifdef SN76489_DATA_PORTB_MASK
	DDRB |= SN76489_DATA_PORTB_MASK;
	#endif
	#ifdef SN76489_DATA_PORTC_MASK
	DDRC |= SN76489_DATA_PORTC_MASK;
	#endif
	#ifdef SN76489_DATA_PORTD_MASK
	DDRD |= SN76489_DATA_PORTD_MASK;
	#endif
	SN76489_WE_PORT |= bit(SN76489_WE_BIT); // HIGH by default
	SN76489_CE_PORT |= bit(SN76489_CE_BIT); // HIGH by default

	/* SN76489 Test code */
	setFrequency(0, 392); // Sol
	setreg(0x01, ATTENUATION_OFF);
	setFrequency(1, 440); // La
	setreg(0x03, ATTENUATION_OFF);
	setFrequency(2, 493); // Si
	setreg(0x05, ATTENUATION_OFF);
	setreg(0x06, NOISE_WHITE_FEEDBACK | NOISE_SHIFT_1024);
	setreg(0x07, ATTENUATION_OFF);
}

void loop_() {
    sleep_enable();
}

void loop() {
	/* Program loop */
	/* SN76489 Test code with synth calls thrown in for good measure. */    
///*    
    delay(1000);
    synth.noteOn(0, 31, 127);
    setreg(0x01, ATTENUATION_8DB);
    delay(1000);
    setreg(0x01, ATTENUATION_OFF);
    delay(1000);
    synth.noteOn(0, 33, 127);
    setreg(0x03, ATTENUATION_8DB);
    delay(1000);
    setreg(0x03, ATTENUATION_OFF);
    delay(1000);
    synth.noteOn(0, 35, 127);
    setreg(0x05, ATTENUATION_8DB);
    delay(1000);
    synth.noteOff(0);
    setreg(0x05, ATTENUATION_OFF);
    delay(1000);
    setreg(0x07, ATTENUATION_2DB);
    delay(1000);
    setreg(0x07, ATTENUATION_OFF);
//*/
    
}


#ifdef USE_QD_PACKETIZER
// ideally this would use a base class/interface instead of function pointer
void qdHelper(const byte *packet) {
    //put your packet parsers here:
    synth.parseMidiPacket(packet);
}


void serialEvent() { // Serial port triggers this when there is data waiting
    qdMidiPacketizer(qdHelper, Serial.read());
}
#else
void serialEvent() { // Serial port triggers this when there is data waiting
    const byte *packet = NULL;
    packet = packetizer.receive(Serial.read());
    //put your packet parsers here:
    synth.parseMidiPacket(packet);
}
#endif






